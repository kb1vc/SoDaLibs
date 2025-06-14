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

#include <SoDa/ReSampler.hxx>
#include <SoDa/NCO.hxx>
#include <SoDa/Format.hxx>
#include <iostream>

int main() {
  int sample_rate = 625000; 
  int out_rate = 48000;
  int in_rate = 625000;
  
  double f_sample_rate = ((double) sample_rate);

  // down sample from 625 to 48
  // aim for about 50 ms. 
  SoDa::ReSampler resamp(in_rate, out_rate, 0.05);

  
  int in_buf_size = resamp.getInputBufferSize();
  int out_buf_size = resamp.getOutputBufferSize();
  
  std::cerr << SoDa::Format("Input buffer size: %0 Output buffer size %1 filter size %2\n")
    .addI(in_buf_size)
    .addI(out_buf_size)
    .addI(resamp.getFilterLength())
    ;
  
  
  // we're going from 625 kS/s to 48 kS/s.  create a 10kHz tone at
  // the higher rate.
  SoDa::NCO  osc(f_sample_rate, 10e3);
  std::vector<std::complex<float>> test_in(in_buf_size);  

  std::vector<std::complex<float>> test_out(out_buf_size);      
  // do this for three buffers worth

  int t_index = 0;
  double out_sample_rate = f_sample_rate * out_rate / in_rate;
  
  double t_incr = 1.0 / out_sample_rate;
  
  for(int i = 0; i < 3; i++) {
    osc.get(test_in);

    // now resample it
    resamp.apply(test_in, test_out);
    
    // write the output
    for(auto v : test_out) {
      std::cout << SoDa::Format("%0 %1 %2\n")
	.addF(double (t_index) * t_incr, 'e')
	.addF(v.real(), 'e')
	.addF(v.imag(), 'e')
	;
      
      t_index++; 
    }
  }
}
