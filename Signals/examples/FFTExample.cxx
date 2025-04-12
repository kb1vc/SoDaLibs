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

#include <iostream>
#include <math.h>
#include <vector>
#include <complex>
#include <SoDa/FFT.hxx>


int main() {
  unsigned int N = 16;
  SoDa::FFT fft(N);

  std::vector<std::complex<float>> in(N), out(N), rev(N);
  
  double angi = 2.0 * M_PI * 4.0 / ((double) N);
  double ang = 0.0;
  double cw[] = {1.0, 0.468, 0.014, 0.003, 0.002, 0.00115, 0.000614, 0.000266, 0,
		 0.000266,  0.000614, 0.00115, 0.002, 0.003, 0.014, 0.468};
  
  for(int i = 0; i < N; i++) {
    ang = ang + angi; 
    //    in[i] = std::complex<float>(cos(ang), 0.0); 
    in[i] = std::complex<float>(cw[i], 0.0);     
  }

		 
  fft.ifft(in, out);
  fft.fft(out, rev);
  
  double out_norm = out[0].real();
  double rev_norm = rev[0].real();
  for(int i = 0; i < N; i++) {
    std::cout << in[i] << " " << out[i].real() / out_norm << " " << rev[i].real() / rev_norm << "\n";
  }


  // now check the shift function.
  for(int i = 6; i < 8; i++) {
    std::vector<std::complex<float>> iv(i), ov(i); 
    for(int j = 0; j < i; j++) {
      iv[j] = std::complex<float>(j, 0);
    }
    fft.shift(iv, iv); 
    for(int j = 0; j < i; j++) {
      std::cout << j << " " << iv[j].real() << "\n";
    }
  }
  
}
