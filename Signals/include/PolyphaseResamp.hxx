#pragma once
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

#include <complex>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>

#include <SoDa/Exception.hxx>

namespace SoDa {
  class PolyphaseResamp;
  typedef std::shared_ptr<PolyphaseResamp> PolyphaseResampPtr;

  /**
   * @class PolyphaseResamp
   *
   * Rational polyphase resampler: converts between any two sample rates
   * whose ratio U:D = (fs_out/gcd):(fs_in/gcd) can be expressed as small integers.
   *
   * The prototype lowpass filter has N_per_phase * U taps split into U polyphase
   * branches.  Each apply() call consumes getInputBufferSize() samples (a multiple
   * of D) and produces getOutputBufferSize() = N_in * U / D samples.
   */
  class PolyphaseResamp {
  public:
    /**
     * @brief Constructor
     *
     * @param input_sample_rate   sample rate of the input stream (Hz)
     * @param output_sample_rate  sample rate of the output stream (Hz)
     * @param n_in                input buffer size override (0 = auto ~50 ms); must be a
     *                            multiple of D = input_sample_rate / gcd(rates).
     *
     * Throws BadSampleRate when the two rates are equal (U == D after gcd reduction).
     * Throws BadBufferSize when n_in > 0 but is not a multiple of D.
     */
    PolyphaseResamp(float input_sample_rate, float output_sample_rate, uint32_t n_in = 0);

    static PolyphaseResampPtr make(float input_sample_rate, float output_sample_rate);

    /** @brief Number of input samples expected per call to apply(). Always a multiple of D. */
    uint32_t getInputBufferSize()  const;

    /** @brief Number of taps per polyphase branch (implementation detail, useful for tuning). */
    uint32_t getNumPhaseTaps() const;

    /** @brief Number of output samples produced per call to apply(). */
    uint32_t getOutputBufferSize() const;

    /**
     * @brief Filter length parameter for SoDa::Checker.
     *
     * Returns round((N_per_phase * U - 1) / D) - 1, encoding the prototype
     * filter's group delay in terms of Checker's phase formula.
     */
    uint32_t getFilterLength() const;

    /** @brief Numerator of the reduced sample-rate ratio (= fs_out / gcd). */
    uint32_t getUpsampleRatio()   const;

    /** @brief Denominator of the reduced sample-rate ratio (= fs_in / gcd). */
    uint32_t getDownsampleRatio() const;

    /** @brief Minimum quantum: input buffer must be a multiple of this (= D). */
    uint32_t quantumSize() const;

    /**
     * @brief Resample a buffer of IQ samples.
     *
     * @param in   input buffer, must have exactly getInputBufferSize() elements
     * @param out  output buffer, must have exactly getOutputBufferSize() elements
     */
    void apply(std::vector<std::complex<float>>& in,
               std::vector<std::complex<float>>& out);

    /** @brief Thrown when input_sample_rate == output_sample_rate (no resampling). */
    class BadSampleRate : public SoDa::Exception {
    public:
      BadSampleRate(float fs_in, float fs_out);
    };

    /** @brief Thrown when apply() receives a buffer of the wrong size. */
    class BadBufferSize : public SoDa::Exception {
    public:
      BadBufferSize(const std::string& which, uint32_t got, uint32_t expected);
    };

  private:
    uint32_t U;           ///< upsample factor   = fs_out / gcd(fs_in, fs_out)
    uint32_t D;           ///< downsample factor  = fs_in  / gcd(fs_in, fs_out)
    uint32_t N_per_phase; ///< taps per polyphase branch
    uint32_t N_in;        ///< input buffer size (multiple of D)
    uint32_t phase_accum; ///< polyphase phase accumulator, 0 .. U+D-1

    std::vector<std::vector<float>> phases;       ///< phases[p][n] = h[n*U + p]
    std::vector<std::complex<float>> delay_line;  ///< history; index 0 = most recent
  };
}
