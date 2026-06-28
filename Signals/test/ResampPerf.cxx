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

// Performance comparison: ReSampler vs PolyphaseResamp vs CascadeResamp.
// Rates: 625kHz -> 48kHz (downsample) and 48kHz -> 625kHz (upsample).
// Push 100 * 300000 samples through each; report wall-clock time.

#include "../include/ReSampler.hxx"
#include "../include/PolyphaseResamp.hxx"
#include "../include/CascadeResamp.hxx"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>
#include <SoDa/Format.hxx>

using CVec  = std::vector<std::complex<float>>;
using Clock = std::chrono::steady_clock;

static double elapsed_ms(Clock::time_point t0) {
  return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

static void fill_noise(CVec& v) {
  for(auto& s : v)
    s = std::complex<float>(float(rand()) / RAND_MAX - 0.5f,
                            float(rand()) / RAND_MAX - 0.5f);
}

// -----------------------------------------------------------------------
static void bench_resampler(float fs_in, float fs_out,
                            uint32_t buf_size, uint32_t total_samples) {
  SoDa::ReSampler resamp(fs_in, fs_out, buf_size);

  CVec in(resamp.getInputBufferSize());
  CVec out(resamp.getOutputBufferSize());
  fill_noise(in);

  uint32_t calls = (total_samples + resamp.getInputBufferSize() - 1)
                   / resamp.getInputBufferSize();

  auto t0 = Clock::now();
  for(uint32_t i = 0; i < calls; i++) resamp.apply(in, out);
  double ms = elapsed_ms(t0);

  std::cout << SoDa::Format("ReSampler        %0->%1  buf_in=%2 buf_out=%3  %4 ms  (%5 Msamples/s)\n")
    .addF(fs_in/1e3,'f').addF(fs_out/1e3,'f')
    .addI(resamp.getInputBufferSize()).addI(resamp.getOutputBufferSize())
    .addF(ms,'f')
    .addF(double(calls) * resamp.getInputBufferSize() / (ms * 1e3), 'f');
}

// -----------------------------------------------------------------------
static void bench_polyphase(float fs_in, float fs_out, uint32_t total_samples) {
  SoDa::PolyphaseResamp resamp(fs_in, fs_out);

  CVec in(resamp.getInputBufferSize());
  CVec out(resamp.getOutputBufferSize());
  fill_noise(in);

  uint32_t calls = (total_samples + resamp.getInputBufferSize() - 1)
                   / resamp.getInputBufferSize();

  auto t0 = Clock::now();
  for(uint32_t i = 0; i < calls; i++) resamp.apply(in, out);
  double ms = elapsed_ms(t0);

  std::cout << SoDa::Format("PolyphaseResamp  %0->%1  buf_in=%2 buf_out=%3  %4 ms  (%5 Msamples/s)"
                             "  [U=%6 D=%7 N_per_phase=%8]\n")
    .addF(fs_in/1e3,'f').addF(fs_out/1e3,'f')
    .addI(resamp.getInputBufferSize()).addI(resamp.getOutputBufferSize())
    .addF(ms,'f')
    .addF(double(calls) * resamp.getInputBufferSize() / (ms * 1e3), 'f')
    .addI(resamp.getUpsampleRatio()).addI(resamp.getDownsampleRatio())
    .addI(resamp.getNumPhaseTaps());
}

// -----------------------------------------------------------------------
static void bench_cascade(float fs_in, float fs_out, uint32_t total_samples) {
  SoDa::CascadeResamp resamp(fs_in, fs_out);

  CVec in(resamp.getInputBufferSize());
  CVec out(resamp.getOutputBufferSize());
  fill_noise(in);

  uint32_t calls = (total_samples + resamp.getInputBufferSize() - 1)
                   / resamp.getInputBufferSize();

  auto t0 = Clock::now();
  for(uint32_t i = 0; i < calls; i++) resamp.apply(in, out);
  double ms = elapsed_ms(t0);

  std::cout << SoDa::Format("CascadeResamp    %0->%1  buf_in=%2 buf_out=%3  %4 ms  (%5 Msamples/s)"
                             "  [%6 stages]\n")
    .addF(fs_in/1e3,'f').addF(fs_out/1e3,'f')
    .addI(resamp.getInputBufferSize()).addI(resamp.getOutputBufferSize())
    .addF(ms,'f')
    .addF(double(calls) * resamp.getInputBufferSize() / (ms * 1e3), 'f')
    .addI(resamp.getNumStages());

  resamp.describe(std::cout);
}

// -----------------------------------------------------------------------
int main() {
  const uint32_t TOTAL = 100 * 300000;
  srand(42);

  std::cout << "=== Downsample: 625 kHz -> 48 kHz  (buf ~50 ms) ===\n";
  bench_resampler (625e3f, 48e3f, 30000, TOTAL);
  bench_polyphase (625e3f, 48e3f,        TOTAL);
  bench_cascade   (625e3f, 48e3f,        TOTAL);

  std::cout << "\n=== Upsample: 48 kHz -> 625 kHz  (buf ~50 ms) ===\n";
  bench_resampler (48e3f, 625e3f, 2304, TOTAL);
  bench_polyphase (48e3f, 625e3f,       TOTAL);
  bench_cascade   (48e3f, 625e3f,       TOTAL);

  std::cout << "\n=== Downsample: 1200 kHz -> 48 kHz  (buf ~50 ms) ===\n";
  bench_resampler (1200e3f, 48e3f, 60000, TOTAL);
  bench_polyphase (1200e3f, 48e3f,        TOTAL);
  bench_cascade   (1200e3f, 48e3f,        TOTAL);

  return 0;
}
