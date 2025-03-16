/*
  Copyright (c) 2022,2025 Matthew H. Reilly (kb1vc)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FFT.hxx"
#include <Utils/include/Format.hxx>
#include <Utils/include/Exception.hxx>

namespace SoDa {

  
  FFT::FFT(unsigned int len, FFTOpt opt) : len(len) {
    fftwf_set_timelimit(1.0);
    
    unsigned int fftw_flag; 

    switch (opt) {
    case MEASURE:
      fftw_flag = FFTW_MEASURE;
      break; 
    case EXHAUST:
      fftw_flag = FFTW_EXHAUSTIVE;
      break; 
    case PATIENT:
      fftw_flag = FFTW_PATIENT;
      break; 
    case ESTIMATE:
    default:
      fftw_flag = FFTW_ESTIMATE; 
      break; 
    }
    
    auto f_dummy_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * len);
    auto f_dummy_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * len);  
  
    forward_plan = fftwf_plan_dft_1d(len, f_dummy_in, f_dummy_out, 
				     FFTW_FORWARD, fftw_flag);
    backward_plan = fftwf_plan_dft_1d(len, f_dummy_in, f_dummy_out, 
				      FFTW_BACKWARD, fftw_flag); // ESTIMATE);

    fftwf_free(f_dummy_in);
    fftwf_free(f_dummy_out);
  }
    
  void FFT::fft(std::vector<std::complex<float>> & in, 
		std::vector<std::complex<float>> & out) {
    if(in.size() != out.size()) {
      throw UnmatchedSizes("fft", in.size(), out.size());
    }
    if(in.size() != len) {
      throw BadSize("fft", in.size(), len);
    }
    
    auto in_p = (fftwf_complex*) in.data();
    auto out_p = (fftwf_complex*) out.data();    
    bool do_fixup = false;
    
    if(fftwf_alignment_of((float*)in_p) != fftwf_alignment_of((float*)out_p)) {
      // buffers are misaligned.  sigh. make a copy
      in_p = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * in.size());
      out_p = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out.size());
      for(int i = 0; i < in.size(); i++) {
	in_p[i][0] = in[i].real();
	in_p[i][1] = in[i].imag();
      }
      do_fixup = true; 
    }
    else {
      in_p = (fftwf_complex*) in.data();
      out_p = (fftwf_complex*) out.data();      
    }
    
    fftwf_execute_dft(forward_plan, in_p, out_p);

    if(do_fixup) {
      for(int i = 0; i < out.size(); i++) {
	out[i] = std::complex<float>(out_p[i][0], out_p[i][1]);
      }
      fftwf_free(in_p);
      fftwf_free(out_p);
    }
  }

  void FFT::ifft(std::vector<std::complex<float>> & in, 
		 std::vector<std::complex<float>> & out) {
    if(in.size() != out.size()) {
      throw UnmatchedSizes("fft", in.size(), out.size());
    }
    if(in.size() != len) {
      throw BadSize("fft", in.size(), len);
    }

    auto in_p = (fftwf_complex*) in.data();
    auto out_p = (fftwf_complex*) out.data();    
    bool do_fixup = false;
    if(fftwf_alignment_of((float*)in_p) != fftwf_alignment_of((float*)out_p)) {
      // buffers are misaligned.  sigh. make a copy
      in_p = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * in.size());
      out_p = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * out.size());
      for(int i = 0; i < in.size(); i++) {
	in_p[i][0] = in[i].real();
	in_p[i][1] = in[i].imag();
      }
      do_fixup = true; 
    }
    
    fftwf_execute_dft(backward_plan, in_p, out_p);

    if(do_fixup) {
      for(int i = 0; i < out.size(); i++) {
	out[i] = std::complex<float>(out_p[i][0], out_p[i][1]);
      }
      fftwf_free(in_p);
      fftwf_free(out_p);
    }
  }
  
  void FFT::shift(std::vector<std::complex<float>> & in, 
		  std::vector<std::complex<float>> & out) {
    // the inputs must be the same size
    if(in.size() != out.size()) {
      throw UnmatchedSizes("shift", in.size(), out.size());
    }
    if((in.size() % 2) == 0) {
      // for the even case, ishift and shift are the
      // same. ishift gets it right....
      return ishift(in, out);
    }
    // take the middle and shift it down
    // since in and out may be the same vectors, we
    // make a copy. (Not a big deal.)
    auto temp = in; 
    unsigned int mid = (in.size() - 1) / 2;
    unsigned int mod = in.size();
    for(int i = 0; i < in.size(); i++) {
      out[(mid + i) % mod] = temp[i];
    }
  }
  
  void FFT::ishift(std::vector<std::complex<float>> & in, 
		   std::vector<std::complex<float>> & out) {
    // the inputs must be the same size
    if(in.size() != out.size()) {
      throw UnmatchedSizes("ishift", in.size(), out.size());
    }
    // take the middle and shift it down
    // the two vectors must be distinct.
    auto temp = in;
    unsigned int mid = (in.size() + 1) / 2;
    unsigned int mod = in.size();
    for(int i = 0; i < in.size(); i++) {
      out[(mid + i) % mod] = temp[i]; 
    }
  }
  
  uint32_t FFT::findGoodSize(uint32_t min_size) {
    // look for the nearest number of the form 2^n * 3^m * 5^p * 7^q
    // that is greater than min_size.
    //
    // The factor of 7 is important for downsampling to 44.1 kHz. 

    // don't go more than 2^x+1 where ceil(log2(min_size)) = x
    int max_n = 0;
    while((1 << max_n) < min_size) max_n++;
    max_n++;
    uint32_t max_val = 4 << max_n;

    uint32_t best_val = max_val;
    
    int n, m, p, q;
    int vn, vm, vp, vq; 
    for(n = 0, vn=1; n < max_n; n++, vn *= 2) {
      for(m = 0, vm = 1; m < 4; m++, vm *= 3) {
	for(p = 0, vp=1; p < 3; p++, vp *= 5) {
	  for(q = 0, vq=1; q < 3; q++, vq *= 7) {
	    auto v = vn * vm * vp * vq; 
	    if((v >= min_size) && (v < best_val)) {
	      best_val = v;
	      if(min_size == v) return v;
	    }
	  }
	}
      }
    }	
    return best_val; 
  }


  FFT::UnmatchedSizes::UnmatchedSizes(const std::string & st, unsigned int ins, unsigned int outs) :
    SoDa::Exception(SoDa::Format("Vector arguments to function FFT::%0 must be the same size. In.size = %1  Out.size = %2\n")
		       .addS(st).addI(ins).addI(outs).str()) { }
    
  FFT::BadSize::BadSize(const std::string & st, unsigned int was, unsigned int should_be) :
    SoDa::Exception(SoDa::Format("Vector arguments to function FFT::%0 must %2 but were %1 instead\n")
		       .addS(st).addI(was).addI(should_be).str()) { }

  std::shared_ptr<FFT> FFT::make(unsigned int len, FFTOpt opt) {
    return std::make_shared<FFT>(len, opt);
  }
}

