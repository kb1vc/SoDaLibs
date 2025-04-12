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

#include "FFT.hxx"
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

int main() {

  int bufsize = 256;
  std::vector<std::complex<float>> iv(bufsize);
  std::vector<std::complex<float>> fv(bufsize);  
  std::vector<std::complex<float>> ov(bufsize);

  SoDa::FFT fft(bufsize);
  
  float f_inc = 0.25; 
  for(float freq = 0.1; freq < 3.0; freq += f_inc) {
    float magerr = 0.0;
    float ang = 0.0;
    float ang_inc = freq;
    for(int i = 0; i < bufsize; i++) {
      iv[i] = std::complex<float>(cos(ang), sin(ang));
      ang = ang + ang_inc; 
    }

    fft.fft(fv, iv);

    // now invert it
    fft.ifft(ov, fv);

    // now plot it
    for(int i = 0; i < bufsize; i++) {
      ov[i] = ov[i] / ((float) bufsize); 
      std::cout << i << " " << iv[i].real() << " " << ov[i].real() << " " << (ov[i].real() - iv[i].real()) << "\n";
    }
  }
  
}
