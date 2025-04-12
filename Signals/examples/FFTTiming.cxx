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

#include <SoDa/FFT.hxx>
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <chrono>

float bump_ang(float a, float ai) {
  a = a + ai;
  if(a > M_PI) a = a - 2*M_PI;

  return a;
}

int main(int argc, char * argv[]) {

  if(argc < 3) {
    std::cerr << "FFTTiming <number-of-trials> <vector-size>\n";
    exit(-1);
  }
  // trials, size 
  int trials = atof(argv[1]);
  int vsize = atof(argv[2]);

  std::vector<std::vector<std::complex<float>>> in_vecs, out_vecs; 

  std::vector<float> ai = { 0.1, 0.13242 };
  std::vector<float> ang = {0.0, 1.2}; 
  // create input buffers and output buffers
  for(int i = 0; i < trials; i++) {
    std::vector<std::complex<float>> iv(vsize);
    std::vector<std::complex<float>> ov(vsize);

    for(int j = 0; j < vsize; j++) {
      iv[j] = std::complex<float>(0.0, 0.0); 
      for(int aii = 0; aii < 2; aii++) {
	ang[aii] = bump_ang(ang[aii], ai[aii]);
	iv[j] += std::complex<float>(sin(ang[aii]), cos(ang[aii]));
      }
    }
    in_vecs.push_back(iv);
    out_vecs.push_back(ov);     
  }

  auto config_start = std::chrono::high_resolution_clock::now();
  // now create the FFT object
  SoDa::FFT fft(vsize, SoDa::FFT::EXHAUST);
  auto config_end = std::chrono::high_resolution_clock::now();
  
  auto config_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(config_end - config_start).count();

  // do the time trial
  auto start = std::chrono::high_resolution_clock::now();

  float sum = 0.0; 
  for(int t = 0; t < trials; t++) {
    fft.fft(in_vecs[t], out_vecs[t]);
    fft.ifft(out_vecs[t], in_vecs[t]); 
    sum += in_vecs[t][t].real();
  }

  auto end = std::chrono::high_resolution_clock::now();
  
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  
  // how many points?
  double points = 2.0 * ((double) vsize) * ((double) trials);
  double elapsed = ns;

  std::cout << "# Vector_Size Config_Time Num_Trials Elapsed_Time Time_per_point\n";
  
  // vsize, trials, elapsed, time-per-pt
  std::cout << vsize
	    << " " << trials
	    << " " << (elapsed * 1e-9)
	    << " " << ((elapsed / points) * 1e-9)
	    << " " << (config_ns * 1e-9)
	    << "\n";
  
  
}
