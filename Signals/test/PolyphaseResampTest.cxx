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

#include "../include/PolyphaseResamp.hxx"
#include <iostream>
#include <cmath>
#include <vector>
#include <complex>
#include <SoDa/Options.hxx>
#include <SoDa/Format.hxx>
#include "../test/Checker.hxx"

static SoDa::Checker::CheckRegion getRegion(double freq, double flo, double fhi, double skirt) {
  if((freq > flo + 0.95 * skirt) && (freq < (fhi - 0.95 * skirt)))
    return SoDa::Checker::PASS_BAND;
  else if((freq < (flo - 1.2 * skirt)) || (freq > (fhi + 1.2 * skirt)))
    return SoDa::Checker::STOP_BAND;
  else
    return SoDa::Checker::TRANSITION_BAND;
}

static bool testResamp(float fs_in, float fs_out) {
  std::cerr << SoDa::Format("PolyphaseResamp %0 -> %1\n").addF(fs_in, 'e').addF(fs_out, 'e');

  SoDa::PolyphaseResamp resamp(fs_in, fs_out);

  std::cerr << SoDa::Format("  U=%0 D=%1 filter_length=%2 in=%3 out=%4\n")
    .addI(resamp.getUpsampleRatio())
    .addI(resamp.getDownsampleRatio())
    .addI(resamp.getFilterLength())
    .addI(resamp.getInputBufferSize())
    .addI(resamp.getOutputBufferSize());

  // Checker is initialized with fs_out directly; the rational resampler produces
  // exactly fs_out (no approximation since U*fs_in/D = fs_out by construction).
  SoDa::Checker chk(double(fs_out),
                    resamp.getFilterLength(),
                    1.0,
                    50.0,
                    0.1,
                    resamp.getInputBufferSize(),
                    resamp.getOutputBufferSize(),
                    double(fs_in),
                    1024);

  double smaller_rate = std::min(double(fs_in), double(fs_out));
  double f_hi   =  smaller_rate * 0.5;
  double f_lo   = -f_hi;
  double skirt  =  0.1 * smaller_rate;

  bool passed = true;
  for(uint32_t i = 0; i < chk.getNumFreqSteps(); i++) {
    chk.checkResponse(i,
      [f_lo, f_hi, skirt](double f) { return getRegion(f, f_lo, f_hi, skirt); },
      [&resamp](std::vector<std::complex<float>>& in,
                std::vector<std::complex<float>>& out) { resamp.apply(in, out); });
    if(!chk.testPassed()) passed = false;
  }

  return passed;
}

static bool testBadRate(float fs_in, float fs_out) {
  std::cerr << SoDa::Format("BadRate %0 -> %1\n").addF(fs_in, 'e').addF(fs_out, 'e');
  try {
    SoDa::PolyphaseResamp resamp(fs_in, fs_out);
    std::cerr << "FAIL: expected BadSampleRate but no exception thrown\n";
    return false;
  }
  catch(const SoDa::PolyphaseResamp::BadSampleRate&) {
    return true;
  }
  catch(...) {
    std::cerr << "FAIL: unexpected exception type\n";
    return false;
  }
}

int main(int argc, char* argv[]) {
  double fs_in, fs_out;

  SoDa::Options cmd;
  cmd.add(&fs_in,  "fsin",  'i', 48e3,  "input sample rate")
     .add(&fs_out, "fsout", 'o', 625e3, "output sample rate")
     .addInfo("Test PolyphaseResamp rational resampler.\n");

  if(!cmd.parse(argc, argv)) {
    std::cout << "Bad command line\nFAILED\n";
    return -1;
  }

  bool passed = true;

  passed = testResamp(float(fs_in), float(fs_out)) && passed;

  // Only equal rates are invalid; up and downsampling are both supported.
  passed = testBadRate(48e3f, 48e3f) && passed;

  std::cout << (passed ? "PASSED" : "FAILED") << "\n";
  return passed ? 0 : 1;
}
