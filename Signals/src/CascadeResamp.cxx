/*
 *  BSD 2-Clause License
 *
 *  Copyright (c) 2025, kb1vc
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "CascadeResamp.hxx"
#include <cmath>
#include <algorithm>
#include <SoDa/Format.hxx>

namespace SoDa {

  static uint32_t ugcd(uint32_t a, uint32_t b) {
    while(b) { uint32_t t = b; b = a % b; a = t; }
    return a;
  }

  // Returns prime factors of n sorted largest-first.
  static std::vector<uint32_t> prime_factors(uint32_t n) {
    std::vector<uint32_t> f;
    for(uint32_t p = 2; uint64_t(p) * p <= uint64_t(n); p++) {
      while(n % p == 0) { f.push_back(p); n /= p; }
    }
    if(n > 1) f.push_back(n);
    std::sort(f.rbegin(), f.rend());
    return f;
  }

  CascadeResampPtr CascadeResamp::make(float fs_in, float fs_out) {
    return std::make_shared<CascadeResamp>(fs_in, fs_out);
  }

  CascadeResamp::CascadeResamp(float fs_in, float fs_out) {
    uint32_t fsi = uint32_t(std::lround(fs_in));
    uint32_t fso = uint32_t(std::lround(fs_out));
    uint32_t g   = ugcd(fsi, fso);
    uint32_t U   = fso / g;
    uint32_t D   = fsi / g;

    if(U == D) throw BadSampleRate(fs_in, fs_out);

    // Stop cascading when the remaining D_rem/U ratio falls at or below this.
    // A ratio of ~4 means N_per_phase is only ~4× the upsampling case — acceptable.
    const double THRESHOLD = 4.0;

    // Build the sequence of intermediate rates.
    // For downsampling: peel off one prime factor of D at a time (largest first)
    // until the remaining ratio D_rem/U is small enough for a single stage.
    rates_.push_back(fs_in);

    if(U < D) {
      auto factors = prime_factors(D);
      uint32_t D_rem   = D;
      float    cur_rate = fs_in;

      for(uint32_t p : factors) {
        if(double(D_rem) / double(U) <= THRESHOLD) break;
        D_rem   /= p;
        cur_rate /= float(p);
        rates_.push_back(cur_rate);
      }

      // After the loop, D_rem:U is the remaining ratio.
      // If D_rem == U the loop fully reduced to 1:1 and cur_rate already equals fs_out;
      // don't add it again.  Otherwise the final rational stage still needs to be added.
      if(D_rem != U) rates_.push_back(fs_out);
    } else {
      // Upsampling: single rational stage.
      rates_.push_back(fs_out);
    }

    // The cascade quantum equals D (same as a single-stage PolyphaseResamp).
    // Each intermediate stage's N_in is determined by propagating from N_in_0.
    uint32_t quantum = D;
    uint32_t target  = std::max(uint32_t(fs_in * 0.05f), quantum);
    N_in_            = ((target + quantum - 1) / quantum) * quantum;

    // Build PolyphaseResamp stages, chaining buffer sizes.
    uint32_t cur_nin = N_in_;
    for(size_t k = 0; k + 1 < rates_.size(); k++) {
      stages_.push_back({});
      Stage& s = stages_.back();
      s.resamp = std::make_unique<PolyphaseResamp>(rates_[k], rates_[k + 1], cur_nin);
      uint32_t next_nin = s.resamp->getOutputBufferSize();
      if(k + 2 < rates_.size())       // intermediate stages need their own buffer
        s.buf.resize(next_nin);
      cur_nin = next_nin;
    }
    N_out_ = cur_nin;
  }

  void CascadeResamp::apply(std::vector<std::complex<float>>& in,
                            std::vector<std::complex<float>>& out) {
    if(uint32_t(in.size())  != N_in_)
      throw BadBufferSize("Input",  uint32_t(in.size()),  N_in_);
    if(uint32_t(out.size()) != N_out_)
      throw BadBufferSize("Output", uint32_t(out.size()), N_out_);

    std::vector<std::complex<float>>* cur_in = &in;
    for(size_t k = 0; k < stages_.size(); k++) {
      auto* cur_out = (k + 1 == stages_.size()) ? &out : &stages_[k].buf;
      stages_[k].resamp->apply(*cur_in, *cur_out);
      cur_in = cur_out;
    }
  }

  void CascadeResamp::describe(std::ostream& os) const {
    for(size_t k = 0; k < stages_.size(); k++) {
      const auto& r = *stages_[k].resamp;
      os << SoDa::Format("  Stage %0: %1 -> %2  U=%3 D=%4 N_per_phase=%5  buf_in=%6 buf_out=%7\n")
        .addI(k)
        .addF(rates_[k], 'e').addF(rates_[k + 1], 'e')
        .addI(r.getUpsampleRatio()).addI(r.getDownsampleRatio())
        .addI(r.getNumPhaseTaps())
        .addI(r.getInputBufferSize()).addI(r.getOutputBufferSize());
    }
  }

  CascadeResamp::BadSampleRate::BadSampleRate(float fs_in, float fs_out) :
    SoDa::Exception(
      SoDa::Format("CascadeResamp: rates are equal or invalid (%0 -> %1)\n")
        .addF(fs_in, 'e').addF(fs_out, 'e').str()) { }

  CascadeResamp::BadBufferSize::BadBufferSize(const std::string& which,
                                              uint32_t got, uint32_t expected) :
    SoDa::Exception(
      SoDa::Format("CascadeResamp: %0 buffer size %1, expected %2\n")
        .addS(which).addI(got).addI(expected).str()) { }

}
