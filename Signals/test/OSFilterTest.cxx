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
#include <fstream>
#include <chrono>

#include "../include/Filter.hxx"
#include "../include/OSFilter.hxx"
#include "../include/NCO.hxx"
#include "Checker.hxx"
#include <Utils/include/Format.hxx>
#include <cmath>

float fixAngle(float a) {
  while(a > M_PI) a = a - 2.0 * M_PI;
  while(a < -M_PI) a = a + 2.0 * M_PI; 
  return a; 
}

SoDa::Checker::CheckRegion getRegion(double freq, double flo, double fhi, double skirt) {
  SoDa::Checker::CheckRegion reg; 
  if((freq > flo + 0.95 * skirt) && (freq < (fhi - 0.95 * skirt))) {
    reg = SoDa::Checker::PASS_BAND; 
    // std::cerr << SoDa::Format("Test freq %0 flo %1 fhi %2 skirt %3\n")
    //   .addF(freq, 'e').addF(flo, 'e').addF(fhi, 'e').addF(skirt, 'e');
  }
  else if((freq < (flo - 1.2 * skirt)) || (freq > (fhi + 1.2 * skirt))) {
    reg = SoDa::Checker::STOP_BAND;       
    // std::cerr << SoDa::Format("Test freq %0 flo %1 fhi %2 skirt %3\n")
    //   .addF(freq, 'e').addF(flo, 'e').addF(fhi, 'e').addF(skirt, 'e');
  }
  else {
    reg = SoDa::Checker::TRANSITION_BAND;             
  }

  return reg; 
}

void dumpGains(SoDa::Checker & chk, 
	       double freq,
	       const std::vector<float> & gainv,
	       const std::vector<float> & phasev) {
  auto fname = SoDa::Format("gain_phase_%0.dat")
    .addF(freq)
    .str()
    ;

  std::ofstream os(fname);

  for(int i = 0; i < gainv.size(); i++) {
    os << chk.getFreq(i) << " " << gainv[i] << " " << phasev[i] << "\n";
  }
}

bool test() {
  double Fs = 48.0e3;
  double flo = -2.0e3;
  double fhi = 10.0e3;
  uint32_t buflen = 2304;
  float skirt = 2.0e3;

  SoDa::OSFilter filt(flo, fhi, skirt, Fs, buflen, 60.0);
  
  auto edges = filt.getFilterEdges(); 
  std::cerr << SoDa::Format("Filter taps %0 fft size %1 edges %2 %3\n")
    .addI(filt.getTaps())
    .addI(filt.getInternalSize())
    .addF(edges.first, 'e')
    .addF(edges.second, 'e');
  
  int num_passes = 6;

  SoDa::Checker chk(Fs, filt.getTaps(), 1.0, 35.0, 0.01, buflen); 

  bool passed = true; 

  for(uint32_t i = 0; i < chk.getNumFreqSteps(); i++) {
    chk.checkResponse(i, [flo, fhi, skirt](double f) { return getRegion(f, flo, fhi, skirt); },
		      [& filt](std::vector<std::complex<float>> & in,
			       std::vector<std::complex<float>> & out) { filt.apply(in, out); });
    
    if(!chk.testPassed()) {
      passed = false; 
    }
  }


  return passed;
}

int main() {
  if(test()) {
    std::cout << "PASSED\n";
  }
  else {
    std::cout << "FAILED\n";
  }
}
