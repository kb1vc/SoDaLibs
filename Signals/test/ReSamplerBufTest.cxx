/*
 *  BSD 2-Clause License
 *
 *  Copyright (c) 2026, kb1vc
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

#include "../include/ReSampler.hxx"
#include <iostream>
#include <cmath>
#include <vector>
#include <complex>
#include <Utils/include/Options.hxx>
#include <Utils/include/Format.hxx>

#include "../test/Checker.hxx"

static SoDa::Checker::CheckRegion getRegion(double freq, double flo, double fhi, double skirt) {
  if((freq > flo + 0.95 * skirt) && (freq < (fhi - 0.95 * skirt)))
    return SoDa::Checker::PASS_BAND;
  else if((freq < (flo - 1.2 * skirt)) || (freq > (fhi + 1.2 * skirt)))
    return SoDa::Checker::STOP_BAND;
  else
    return SoDa::Checker::TRANSITION_BAND;
}

// Test that construction with a valid (multiple-of-quantum) buffer size works:
//   - getInputBufferSize() returns exactly the requested size
//   - signal quality passes the same criteria as ReSamplerTest
static bool testCorrectSize(float fs_in, float fs_out) {
  uint32_t quantum = SoDa::ReSampler::quantumSize(fs_in, fs_out);

  // pick a buffer size close to 50 ms at fs_in, rounded up to a multiple of quantum
  uint32_t approx = uint32_t(fs_in * 0.05f);
  uint32_t k = (approx + quantum - 1) / quantum;
  if(k < 10) k = 10;
  uint32_t buf_size = k * quantum;

  std::cerr << SoDa::Format("CorrectSize %0 -> %1: quantum=%2 buf_size=%3\n")
    .addF(fs_in, 'e').addF(fs_out, 'e').addI(quantum).addI(buf_size);

  SoDa::ReSampler resamp(fs_in, fs_out, buf_size);

  if(resamp.getInputBufferSize() != buf_size) {
    std::cerr << SoDa::Format("FAIL: getInputBufferSize()=%0 expected %1\n")
      .addI(resamp.getInputBufferSize()).addI(buf_size);
    return false;
  }

  SoDa::Checker chk(fs_out,
                    resamp.getFilterLength(),
                    1.0,
                    50.0,
                    0.1,
                    resamp.getInputBufferSize(),
                    resamp.getOutputBufferSize(),
                    fs_in,
                    1024);

  double smaller_rate = std::min(double(fs_in), double(fs_out));
  double f_hi = smaller_rate * 0.5;
  double f_lo = -f_hi;
  double skirt = 0.1 * smaller_rate;

  bool passed = true;
  for(uint32_t i = 0; i < chk.getNumFreqSteps(); i++) {
    chk.checkResponse(i,
      [f_lo, f_hi, skirt](double f) { return getRegion(f, f_lo, f_hi, skirt); },
      [&resamp](std::vector<std::complex<float>> & in,
                std::vector<std::complex<float>> & out) { resamp.apply(in, out); });
    if(!chk.testPassed()) passed = false;
  }

  return passed;
}

// Test that construction with a size that is NOT a multiple of quantum throws BadQuantum.
// When quantum == 1 every size is valid, so the exception test is skipped.
static bool testBadQuantum(float fs_in, float fs_out) {
  uint32_t quantum = SoDa::ReSampler::quantumSize(fs_in, fs_out);

  if(quantum == 1) {
    std::cerr << SoDa::Format("BadQuantum %0 -> %1: quantum=1, all sizes valid, skipped\n")
      .addF(fs_in, 'e').addF(fs_out, 'e');
    return true;
  }

  uint32_t bad_size = quantum + 1; // (quantum+1) % quantum == 1, never zero

  std::cerr << SoDa::Format("BadQuantum %0 -> %1: quantum=%2 bad_size=%3\n")
    .addF(fs_in, 'e').addF(fs_out, 'e').addI(quantum).addI(bad_size);

  try {
    SoDa::ReSampler resamp(fs_in, fs_out, bad_size);
    std::cerr << SoDa::Format("FAIL: expected BadQuantum for size=%0 quantum=%1 but no exception thrown\n")
      .addI(bad_size).addI(quantum);
    return false;
  }
  catch(const SoDa::ReSampler::BadQuantum &) {
    return true;
  }
  catch(...) {
    std::cerr << "FAIL: unexpected exception type thrown instead of BadQuantum\n";
    return false;
  }
}

// Test construction with a caller-specified buffer size and verify both buffer
// sizes exactly.  Optionally checks signal quality via Checker.
static bool testExactSizes(float fs_in, float fs_out,
                           uint32_t in_size, uint32_t expected_out) {
  std::cerr << SoDa::Format("ExactSize %0 -> %1: in=%2 expected_out=%3\n")
    .addF(fs_in, 'e').addF(fs_out, 'e').addI(in_size).addI(expected_out);

  SoDa::ReSampler resamp(fs_in, fs_out, in_size);

  bool passed = true;

  if(resamp.getInputBufferSize() != in_size) {
    std::cerr << SoDa::Format("FAIL: getInputBufferSize()=%0 expected %1\n")
      .addI(resamp.getInputBufferSize()).addI(in_size);
    passed = false;
  }

  if(resamp.getOutputBufferSize() != expected_out) {
    std::cerr << SoDa::Format("FAIL: getOutputBufferSize()=%0 expected %1\n")
      .addI(resamp.getOutputBufferSize()).addI(expected_out);
    passed = false;
  }

  if(!passed) return false;

  SoDa::Checker chk(fs_out,
                    resamp.getFilterLength(),
                    1.0,
                    50.0,
                    0.1,
                    resamp.getInputBufferSize(),
                    resamp.getOutputBufferSize(),
                    fs_in,
                    1024);

  double smaller_rate = std::min(double(fs_in), double(fs_out));
  double f_hi = smaller_rate * 0.5;
  double f_lo = -f_hi;
  double skirt = 0.1 * smaller_rate;

  for(uint32_t i = 0; i < chk.getNumFreqSteps(); i++) {
    chk.checkResponse(i,
      [f_lo, f_hi, skirt](double f) { return getRegion(f, f_lo, f_hi, skirt); },
      [&resamp](std::vector<std::complex<float>> & in,
                std::vector<std::complex<float>> & out) { resamp.apply(in, out); });
    if(!chk.testPassed()) passed = false;
  }

  return passed;
}

int main(int argc, char * argv[]) {
  double fs_in, fs_out;
  int buf_size_arg = 0;
  int expected_out_arg = 0;

  SoDa::Options cmd;
  cmd.add(&fs_in, "fsin", 'i', 48e3, "input sample rate")
     .add(&fs_out, "fsout", 'o', 8e3, "output sample rate")
     .add(&buf_size_arg, "bufsize", 'b', 0, "exact input buffer size (0 = auto)")
     .add(&expected_out_arg, "expectedout", 'e', 0, "expected output buffer size (checked when non-zero)")
     .addInfo("Test ReSampler constructed with an exact input buffer size.\n"
              "Verifies getInputBufferSize() exactness and BadQuantum exception.\n"
              "With --bufsize N: also verifies getOutputBufferSize() == --expectedout.\n");

  if(!cmd.parse(argc, argv)) {
    std::cout << "Bad command line\nFAILED\n";
    return -1;
  }

  bool passed = true;

  if(buf_size_arg > 0) {
    passed = testExactSizes(float(fs_in), float(fs_out),
                            uint32_t(buf_size_arg),
                            uint32_t(expected_out_arg)) && passed;
  } else {
    passed = testCorrectSize(float(fs_in), float(fs_out)) && passed;
  }

  passed = testBadQuantum(float(fs_in), float(fs_out)) && passed;

  std::cout << (passed ? "PASSED" : "FAILED") << "\n";
  return passed ? 0 : 1;
}
