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

#include "ReSampler.hxx"

#include <iostream>
#include <fstream>
#include <Utils/include/Format.hxx>


namespace SoDa {
  static uint32_t getGCD(uint32_t a, uint32_t b) {
    if(b == 0) return a;
    else if(a < b) {
      return getGCD(b, a);
    }
    else {
      return getGCD(b, a % b);
    }
  }

  // Compute the overlap-and-save save_count from filter design parameters.
  // save_count depends only on sample rates and D, never on buffer size.
  static uint32_t calcSaveCount(float FS_in, float FS_out, uint32_t D) {
    double passband = std::min(FS_in, FS_out);
    double skirt_proportion = std::max(FS_in, FS_out) / (passband * (0.5 - 0.45));
    uint32_t num_taps = uint32_t(0.5 + skirt_proportion * 60.0 / 22.0);
    if((num_taps % 2) == 0) num_taps++;
    if(num_taps < 121) num_taps = 121;
    uint32_t sc = uint32_t((int(num_taps) + int(D) - 1) / int(D)) * D;
    if(sc < num_taps + 1) sc += D;
    return sc;
  }

  uint32_t ReSampler::quantumSize(float FS_in, float FS_out) {
    uint32_t i_fs_in = uint32_t(FS_in);
    uint32_t i_fs_out = uint32_t(FS_out);
    return i_fs_in / getGCD(i_fs_in, i_fs_out);
  }

  ReSamplerPtr ReSampler::make(float FS_in, float FS_out, float time_span_min) {
    return std::make_shared<ReSampler>(FS_in, FS_out, time_span_min);
  }

  ReSamplerPtr ReSampler::make(float FS_in, float FS_out, uint32_t input_buffer_size) {
    return std::make_shared<ReSampler>(FS_in, FS_out, input_buffer_size);
  }

  void ReSampler::setup(float FS_in, float FS_out, uint32_t lx) {
    // lx is the full FFT buffer size and must be a multiple of D (already set).
    double corner_factor = 0.45;
    double passband = std::min(FS_in, FS_out);
    double cutoff = passband * corner_factor;

    double skirt_proportion = std::max(FS_in, FS_out) / (passband * (0.5 - corner_factor));
    uint32_t num_taps = uint32_t(0.5 + skirt_proportion * 60.0 / 22.0);
    if((num_taps % 2) == 0) num_taps++;
    if(num_taps < 121) num_taps = 121;

    scale_factor = float(D) / float(U);

    int savek = (int(num_taps) + int(D) - 1) / int(D);
    save_count = uint32_t(savek) * D;
    if(save_count < (num_taps + 1)) save_count += D;

    discard_count = save_count * U / D;
    num_taps = save_count + 1;

    Lx = lx;
    Ly = Lx * U / D;

    if(FS_out > FS_in) {
      float up_ratio = float(FS_out / FS_in);
      lpf_p = std::make_unique<SoDa::Filter>(-cutoff, cutoff,
					     0.015 * cutoff,
					     FS_out,
					     num_taps, Ly,
					     up_ratio);
    }
    else {
      lpf_p = std::make_unique<SoDa::Filter>(-cutoff, cutoff,
					     0.015 * cutoff,
					     FS_in,
					     num_taps, Lx);
    }

    x.resize(Lx);
    y.resize(Ly);
    X.resize(Lx);
    Y.resize(Ly);

    for(auto & s : x) s = std::complex<float>(0.0, 0.0);
    for(auto & s : Y) s = std::complex<float>(0.0, 0.0);

    in_fft_p = std::make_unique<SoDa::FFT>(Lx);
    out_fft_p = std::make_unique<SoDa::FFT>(Ly);
  }

  ReSampler::ReSampler(float FS_in, float FS_out, float time_span_min) {
    uint32_t i_fs_in = uint32_t(FS_in);
    uint32_t i_fs_out = uint32_t(FS_out);
    auto gcd = getGCD(i_fs_in, i_fs_out);
    U = i_fs_out / gcd;
    D = i_fs_in / gcd;

    uint32_t min_in_samples = uint32_t(FS_in * time_span_min);
    uint32_t min_out_samples = (U * min_in_samples) / D;
    while((min_in_samples < 1000) || (min_out_samples < 1000)) {
      min_in_samples += 1000;
      min_out_samples = (U * min_in_samples) / D;
    }

    auto k = (min_in_samples + D - 1) / D;
    setup(FS_in, FS_out, k * D);
  }

  ReSampler::ReSampler(float FS_in, float FS_out, uint32_t input_buffer_size) {
    uint32_t i_fs_in = uint32_t(FS_in);
    uint32_t i_fs_out = uint32_t(FS_out);
    auto gcd = getGCD(i_fs_in, i_fs_out);
    U = i_fs_out / gcd;
    D = i_fs_in / gcd;

    if(input_buffer_size % D != 0) {
      throw BadQuantum(input_buffer_size, D);
    }

    // save_count depends only on sample rates and D, so we can compute it
    // before calling setup to derive the full FFT buffer size lx.
    uint32_t sc = calcSaveCount(FS_in, FS_out, D);
    setup(FS_in, FS_out, input_buffer_size + sc);
  }


  ReSampler::~ReSampler() {
  }

  uint32_t ReSampler::getInputBufferSize() {
    return Lx - save_count;
  }

  
  uint32_t ReSampler::getOutputBufferSize() {
    return Ly - discard_count;
  }

  uint32_t ReSampler::getFilterLength() { 
    return save_count + 1;
  }
  
  uint32_t ReSampler::apply(std::vector<std::complex<float>> & in,
			    std::vector<std::complex<float>> & out) {
    if(in.size() != getInputBufferSize()) {
      throw BadBufferSize("Input", in.size(), getInputBufferSize());
    }

    if(out.size() != getOutputBufferSize()) {
      throw BadBufferSize("Output", out.size(), getOutputBufferSize());
    }

    // first do the overlap-and-save thing.
    for(int i = 0; i < save_count; i++) {
      x.at(i) = x.at(x.size() - save_count + i);
    }
    for(int i = save_count; i < Lx; i++) {
      x.at(i) = in.at(i - save_count); 
    }
    
    // now do the FFT
    in_fft_p->fft(x, X);

    // apply the filter -- we must do this
    // even if we are upsampling, as a raw copy of
    // an FFT segment looks like a brick wall otherwise. 
    // if we are downsampling, apply the lpf here. 

      
    // now load the output Y vector
    if(Y.size() < X.size()) {
      // we are downsampling.
      // trim out the potential aliasing components
      lpf_p->apply(X, X, Filter::InOutMode(false,false));            
      if(Ly == 0) return 0;
      uint32_t y_half_count = ((Ly + 1) / 2);
      for(uint32_t i = 0; i < y_half_count - 1; i++) {
	Y.at(i) = X.at(i);
	Y.at(Ly - 1 - i) = X.at(Lx - 1 - i);
      }
      Y.at(y_half_count) = X.at(y_half_count);
    }
    else {
      // we are up sampling. Y gets half of X in the bottom, half in the top.
      // we also need to do a gain correction. 
      for(int i = 0; i < Lx; i++) {
	if(i < Lx/2) {
	  // we're on the DC and above side.
	  Y.at(i) = X.at(i);
	}
	else {
	  auto yi = (Ly - 1) - (Lx - 1) + i;
	  Y.at((Ly - 1) -(Lx - 1) + i) = X.at(i);
	}
      }

      /// oooh... we're stuffed.  apply the LPF
      lpf_p->apply(Y, Y, Filter::InOutMode(false, false));
    }

    // do the inverse FFT
    out_fft_p->ifft(Y, y);
      
    // and copy to the output
    for(size_t i = 0; i < getOutputBufferSize(); i++) {
      out.at(i) = y.at(i + discard_count);
    }

    // and that's it!
    return 0;
  }

  uint32_t ReSampler::apply(std::vector<float> & in,
			    std::vector<float> & out) {
    // not exactly optimal, but what were people thinking anyway?
    std::vector<std::complex<float>> tin(in.size());
    std::vector<std::complex<float>> tout(out.size());    
    for(size_t i = 0; i < in.size(); i++) {
      tin[i] = std::complex<float>(in[i], 0.0);
    }
    apply(tin, tout);
    for(size_t i = 0; i < out.size(); i++) {
      out[i] = tout[i].real();
    }

    return out.size();
    
  }

  ReSampler::BadBufferSize::BadBufferSize(const std::string & st, uint32_t got_size, uint32_t should_be_size) :
	SoDa::Exception(SoDa::Format("ReSampler::BadBufferSize:: %0 buffer was length %1 should have been %2\n")
			   .addS(st)
			   .addI(got_size)
			   .addI(should_be_size)
			   .str()) { }

  ReSampler::BadQuantum::BadQuantum(uint32_t input_buffer_size, uint32_t quantum) :
	SoDa::Exception(SoDa::Format("ReSampler::BadQuantum: input_buffer_size %0 must be a multiple of %1\n")
			   .addI(input_buffer_size)
			   .addI(quantum)
			   .str()) { }
  
}
