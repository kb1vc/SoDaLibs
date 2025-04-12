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


#include <cstdint>
#include <memory>
#ifdef SODA_LIB_BUILD
#include <Utils/include/Exception.hxx>
#else
#include <SoDa/Exception.hxx>
#endif

///
///  @file FFT.hxx
///  @brief General wrapper for fftw3 or whatever DFT widget we're going to use.
///  Inspired by the numpy fft functions. Limitied to "float" types. 
///
///  @author M. H. Reilly (kb1vc)
///  @date   Mar, 2025
///

#include <complex>
#include <vector>
#include <fftw3.h>
#include <stdexcept>

namespace SoDa {
  /**
   * @class FFT
   *
   * @brief a low-pain, high-gain interface to the FFTW library. 
   */
  class FFT {
  public:
    /**
     * @class UnmatchedSizes
     *
     * @brief a subclass of SoDa::Exception thrown when the input and output buffer sizes
     * passed to FFT::fft or FFT::ifft don't match. 
     */
    class UnmatchedSizes : public SoDa::Exception {
    public:
      /**
       * @brief constructor
       *
       * @param st a string describing the problem and whence it came.
       * @param ins the size of the input vector
       * @param outs the size of the output vector
       */
      UnmatchedSizes(const std::string & st, unsigned int ins, unsigned int outs);
    };
    
    /**
     * @class BadSize
     *
     * @brief a subclass of SoDa::Exception thrown when the input buffer size does not
     * match the original size passed to the constructor. 
     */
    class BadSize : public SoDa::Exception {
    public:
      /**
       * @brief constructor
       *
       * @param st a string describing the problem and whence it came.
       * @param was the size of the vector passed to FFTW
       * @param should_be the size of the vector expected by this instance of FFT
       */
      BadSize(const std::string & st, unsigned int was, unsigned int should_be); 
    };

    ///  Select the level of optimization for fft/ifft operations
    enum FFTOpt {
      ESTIMATE, ///< Take a stab at optimum settings. This is generally pretty good.
      MEASURE,  ///< Do some tests on a few relevant sizes
      PATIENT,  ///< Like MEASURE but it tries a few more sizes. Wait a little while. 
      EXHAUST   ///< Go get a cup of coffee. This is going to look at a lot of alternatives. Likely not worth it.
      
    };      

    /**
     * @brief the constructor
     *
     * @param len the length of the buffer on which this FFT will operate.
     * The buffer length is fixed for all time. That's the way fftw works -- it
     * needs to know the buffer sizes in order to optimize the operation.
     * @param opt select how aggressive fftw will be in its attempt to
     * optimize fft and ifft. See ::FFTOpt.
     */
    FFT(unsigned int len, FFTOpt opt = ESTIMATE); 

    /**
     * @brief Find a we-hope-is-close-to-optimal size for the operation
     * The buffer size is of the form 2^n * 3^m * 5^p * 7^q -- The factors
     * are sufficient for most interesting radio sample rates, and for audio
     * rates that are multiples of 48 kHz and 44.1 kHz. 
     *
     * @param min_size the minimum size for the buffer
     * than min_size and that is of the form 2^n * 3^m * 5^p * 7^q
     */
    static uint32_t findGoodSize(uint32_t min_size);
    
    /**
     * @brief Perform a forward DFT.
     *
     *
     * @param in the input buffer
     * @param out the output buffer
     *
     * Throws BadSize and UnmatchedSizes if it is annoyed.     
     * 
     */
    void fft(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

    /**
     * @brief Perform an inverse DFT.
     *
     *
     * @param in the input buffer
     * @param out the output buffer
     *
     * Throws BadSize and UnmatchedSizes.
     * 
     */
    void ifft(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

    /**
     * @brief Shifts the input vector from "fft order" to "spectrum order."
     *
     * @param in the input buffer
     * @param out the output buffer
     *
     * An FFT operation will produce a vector who's elements run from
     * DC to f_max, then from -f_max to DC. Spectrum order makes this
     * compatible with humans (and simplifies some code) where the
     * elements run from -f_max to +f_max. 
     *
     * Throws UnmatchedSizes.
     * 
     */
    void shift(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);

    /**
     * @brief Shifts the input vector from "spectrum order" to "fft order."
     *
     * @param in the input buffer
     * @param out the output buffer
     *
     * An FFT operation will produce a vector who's elements run from
     * DC to f_max, then from -f_max to DC. Spectrum order makes this
     * compatible with humans (and simplifies some code) where the
     * elements run from -f_max to +f_max. 
     *
     * Throws UnmatchedSizes.
     * 
     */
    void ishift(std::vector<std::complex<float>> & in, 
	     std::vector<std::complex<float>> & out);



    /**
     * @brief create a pointer to an FFT object
     *
     * @param len the length of the buffer on which this FFT will operate.
     * The buffer length is fixed for all time. That's the way fftw works -- it
     * needs to know the buffer sizes in order to optimize the operation.
     * @param opt select how aggressive fftw will be in its attempt to
     * optimize fft and ifft. See ::FFTOpt.
     * @return a shared pointer to an FFT widget.
     */
    static std::shared_ptr<FFT> make(unsigned int len, FFTOpt opt = ESTIMATE); 
    
  protected:
    
    fftwf_plan forward_plan; ///< fftw maintains a "plan" that contains the optimization information. 
    fftwf_plan backward_plan; ///< the optimization information for the reverse fft
    
    unsigned int len; ///< the required length for input and output operands. 
  };
}

