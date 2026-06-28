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

#include "PolyphaseResamp.hxx"
#include <cmath>
#include <algorithm>
#include <SoDa/Format.hxx>

namespace SoDa {

  static uint32_t ugcd(uint32_t a, uint32_t b) {
    while(b) { uint32_t t = b; b = a % b; a = t; }
    return a;
  }

  PolyphaseResampPtr PolyphaseResamp::make(float fs_in, float fs_out) {
    return std::make_shared<PolyphaseResamp>(fs_in, fs_out);
  }

  PolyphaseResamp::PolyphaseResamp(float fs_in, float fs_out, uint32_t n_in_override) {
    uint32_t fsi = uint32_t(std::lround(fs_in));
    uint32_t fso = uint32_t(std::lround(fs_out));
    uint32_t g   = ugcd(fsi, fso);
    U = fso / g;
    D = fsi / g;

    if(U == D) throw BadSampleRate(fs_in, fs_out);

    // Harris formula: attenuation A_dB at normalized bandwidth Δf relative to virtual rate.
    // Δf = 0.1 * min(fs_in,fs_out) / (U * fs_in) = 0.1 / max(U, D).
    // N_total ≈ A_dB / (22 * Δf) = A_dB * max(U,D) / 2.2.
    // N_per_phase = ceil(N_total / U), rounded up to odd for symmetric center.
    uint32_t maxUD = std::max(U, D);
    uint32_t N_total_ideal = uint32_t(std::ceil(80.0 * maxUD / 2.2));
    N_per_phase = (N_total_ideal + U - 1) / U;
    if(N_per_phase < 3) N_per_phase = 3;
    if(N_per_phase % 2 == 0) N_per_phase++;

    uint32_t N_total = N_per_phase * U;
    double   M       = (double(N_total) - 1.0) / 2.0;

    // Prototype windowed-sinc, cutoff = 0.45 * min(fs_in, fs_out).
    // In virtual-rate normalized units: cutoff = 0.45 / max(U,D).
    // Sinc argument: 0.9 * (k - M) / max(U, D).
    std::vector<double> h(N_total);
    for(uint32_t k = 0; k < N_total; k++) {
      double x        = 0.9 * (double(k) - M) / double(maxUD);
      double sinc_val = (std::abs(x) < 1e-10) ? 1.0 : std::sin(M_PI * x) / (M_PI * x);
      double bk       = 0.42
                        - 0.5  * std::cos(2.0 * M_PI * k / (N_total - 1))
                        + 0.08 * std::cos(4.0 * M_PI * k / (N_total - 1));
      h[k] = sinc_val * bk;
    }

    // Normalize so each polyphase branch sums to 1: total sum = U.
    double total = 0.0;
    for(auto v : h) total += v;
    double scale = double(U) / total;
    for(auto& v : h) v *= scale;

    // Polyphase decomposition: phases[p][n] = h[n*U + p].
    phases.resize(U);
    for(uint32_t p = 0; p < U; p++) {
      phases[p].resize(N_per_phase);
      for(uint32_t n = 0; n < N_per_phase; n++)
        phases[p][n] = float(h[n * U + p]);
    }

    delay_line.assign(N_per_phase, std::complex<float>(0.0f, 0.0f));
    phase_accum = 0;

    if(n_in_override == 0) {
      // Default: ~50 ms at fs_in, rounded up to a multiple of D.
      uint32_t target = std::max(uint32_t(fs_in * 0.05f), D);
      N_in = ((target + D - 1) / D) * D;
    } else {
      if(n_in_override % D != 0)
        throw BadBufferSize("N_in override", n_in_override,
                            ((n_in_override + D - 1) / D) * D);
      N_in = n_in_override;
    }
  }

  uint32_t PolyphaseResamp::getInputBufferSize()  const { return N_in; }
  uint32_t PolyphaseResamp::getNumPhaseTaps()     const { return N_per_phase; }
  uint32_t PolyphaseResamp::getOutputBufferSize() const { return (N_in / D) * U; }
  uint32_t PolyphaseResamp::getUpsampleRatio()    const { return U; }
  uint32_t PolyphaseResamp::getDownsampleRatio()  const { return D; }
  uint32_t PolyphaseResamp::quantumSize()         const { return D; }

  uint32_t PolyphaseResamp::getFilterLength() const {
    // Group delay = (N_per_phase*U - 1) / 2 virtual samples = (N_per_phase*U-1)/(2D) output samples.
    // Checker: phase = -(f/high_rate)*(FL+1)*π, high_rate = max(fs_in, fs_out).
    // Setting equal to -2π*f*τ/fs_out:
    //   (FL+1) = 2 * high_rate * τ / fs_out
    //          = 2 * high_rate * (N_total-1) / (2D * fs_out).
    // For upsampling   (U>D): high_rate=fs_out       → FL+1 = (N_total-1)/D.
    // For downsampling (D>U): high_rate=fs_in=D*fs_out/U → FL+1 = (N_total-1)/U.
    // Unified: FL = round((N_per_phase*U - 1) / min(U,D)) - 1.
    uint32_t minUD = std::min(U, D);
    return uint32_t(std::lround(double(N_per_phase * U - 1) / double(minUD))) - 1;
  }

  void PolyphaseResamp::apply(std::vector<std::complex<float>>& in,
                              std::vector<std::complex<float>>& out) {
    uint32_t N_out = (N_in / D) * U;
    if(uint32_t(in.size())  != N_in)  throw BadBufferSize("Input",  uint32_t(in.size()),  N_in);
    if(uint32_t(out.size()) != N_out) throw BadBufferSize("Output", uint32_t(out.size()), N_out);

    uint32_t j = 0;  // output index

    for(uint32_t i = 0; i < N_in; i++) {
      // Shift delay line (most recent sample at index 0).
      for(uint32_t n = N_per_phase - 1; n > 0; n--)
        delay_line[n] = delay_line[n - 1];
      delay_line[0] = in[i];

      // Generate all output samples whose filter center is at this input.
      // phase_accum < U means this input is the center for the next output.
      while(phase_accum < U) {
        std::complex<float> sum(0.0f, 0.0f);
        const float* ph = phases[phase_accum].data();
        for(uint32_t n = 0; n < N_per_phase; n++)
          sum += ph[n] * delay_line[n];
        out[j++] = sum;
        phase_accum += D;
      }
      phase_accum -= U;
    }
  }

  PolyphaseResamp::BadSampleRate::BadSampleRate(float fs_in, float fs_out) :
    SoDa::Exception(
      SoDa::Format("PolyphaseResamp: input and output sample rates are equal (%0 == %1)\n")
        .addF(fs_in, 'e').addF(fs_out, 'e').str()) { }

  PolyphaseResamp::BadBufferSize::BadBufferSize(const std::string& which,
                                                uint32_t got, uint32_t expected) :
    SoDa::Exception(
      SoDa::Format("PolyphaseResamp: %0 buffer size %1, expected %2\n")
        .addS(which).addI(got).addI(expected).str()) { }

}
