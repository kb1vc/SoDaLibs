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

#include "PolyphaseResamp.hxx"
#include <complex>
#include <vector>
#include <memory>
#include <string>
#include <ostream>

#include <SoDa/Exception.hxx>

namespace SoDa {
  class CascadeResamp;
  typedef std::shared_ptr<CascadeResamp> CascadeResampPtr;

  /**
   * @class CascadeResamp
   *
   * Multi-stage rational polyphase resampler.
   *
   * For large downsampling ratios the direct PolyphaseResamp requires a very
   * long delay line (N_per_phase ∝ D/U), making every input sample expensive.
   * CascadeResamp factors D into small prime stages so each stage's filter is
   * short, then applies a final rational stage for the remaining ratio.
   *
   * Example — 625 kHz → 48 kHz  (D=625, U=48):
   *   Stage 0:  625 kHz → 125 kHz  (÷5,  N_per_phase ≈ 182)
   *   Stage 1:  125 kHz →  48 kHz  (48:125, N_per_phase ≈ 95)
   *
   * The interface is identical to PolyphaseResamp.
   */
  class CascadeResamp {
  public:
    /**
     * @brief Constructor.  Automatically builds the cascade.
     *
     * @param input_sample_rate   Hz
     * @param output_sample_rate  Hz
     *
     * Throws BadSampleRate when the two rates are equal.
     */
    CascadeResamp(float input_sample_rate, float output_sample_rate);

    static CascadeResampPtr make(float input_sample_rate, float output_sample_rate);

    /** @brief Number of input samples consumed per apply() call. */
    uint32_t getInputBufferSize()  const { return N_in_; }

    /** @brief Number of output samples produced per apply() call. */
    uint32_t getOutputBufferSize() const { return N_out_; }

    /** @brief Number of cascade stages built. */
    uint32_t getNumStages() const { return uint32_t(stages_.size()); }

    /**
     * @brief Resample a buffer of IQ samples through the cascade.
     *
     * @param in   input buffer, must have exactly getInputBufferSize() elements
     * @param out  output buffer, must have exactly getOutputBufferSize() elements
     */
    void apply(std::vector<std::complex<float>>& in,
               std::vector<std::complex<float>>& out);

    /** @brief Print a one-line summary per stage to os. */
    void describe(std::ostream& os) const;

    class BadSampleRate : public SoDa::Exception {
    public: BadSampleRate(float fs_in, float fs_out);
    };
    class BadBufferSize : public SoDa::Exception {
    public: BadBufferSize(const std::string& which, uint32_t got, uint32_t expected);
    };

  private:
    struct Stage {
      std::unique_ptr<PolyphaseResamp> resamp;
      std::vector<std::complex<float>> buf;  // output/input buffer between stages
    };

    std::vector<Stage>  stages_;
    std::vector<float>  rates_;   ///< [fs_in, intermediate..., fs_out]
    uint32_t            N_in_;
    uint32_t            N_out_;
  };
}
