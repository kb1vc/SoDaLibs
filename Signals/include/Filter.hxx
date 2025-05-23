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


///
///  @file Filter.hxx
///  @brief This class creates an impulse response and a frequency image for
///  a filter given the specification, number of taps, resulting image size,
///  and sample rate.
///  It can be directly applied to an input buffer. 
///
///  This scheme replaces the awful filter creation scheme in SoDaRadio versions
///  8.x and before. 
///
///  @author M. H. Reilly (kb1vc)
///  @date   July 2022
///

#include <memory>
#include <iostream>
#include <complex>
#include <vector>
#include <fftw3.h>
#include "FilterSpec.hxx"
#include "FFT.hxx"
#include <stdexcept>

namespace SoDa {

    // all "apply" functions can bypass one or more of the input and output conversions
  // into the frequency domain. 
  
  class Filter {
  public:
    /**
     * @class BadBufferSize
     *
     * @brief The filter was built to process a buffer of a size different from the
     * one that was passed to "apply."
     */
    class BadBufferSize : public std::runtime_error {
    public:
      /**
       * @brief Signal a buffer size mismatch error.
       *
       * @param st a more detailed description of what happened.
       * @param in the input buffer size
       * @param out the output buffer size
       * @param req the buffer size for which the filter was created.
       */
      BadBufferSize(const std::string & st, unsigned int in, unsigned int out, unsigned int req);
    };


    /**
     * @class InOutMode
     * @brief indicate to filter apply method that input is already in frequency domain, or output
     * should be left in frequency domain
     */
    struct InOutMode {
      /**
       * @param ti if true, the input is already in the frequency domain
       * @param to if true, do not transform the filter output from the frequency domain
       */
      InOutMode(bool ti, bool to) : xform_in(ti), xform_out(to) { }
      bool xform_in;
      bool xform_out; 
    };

    /// select the window to be applied in constructing the filter
    enum WindowChoice {
      NOWINDOW, ///< a really bad idea. Included just for experimentation
      HAMMING, ///< a classic
      HANN, ///< tight skirts (!)
      BLACKMAN ///< good compromise
    };
    
    /**
     * @brief Build the filter from a filter spec for a general filter
     *
     * @param filter_spec object of class FilterSpec identifying corner frequencies and amplitudes
     * @param buffer_size the impulse response and frequency image will be this long
     * @param gain passband gain (max gain) through filter
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN 
     */
    Filter(FilterSpec & filter_spec, 
	   unsigned int buffer_size, 
	   float gain = 1.0, 
	   WindowChoice window = HANN);        

    /**
     * @brief Alternate constructor, for very simple filters
     *
     * @param low_cutoff lower edge of the filter
     * @param high_cutoff upper edge of the filter
     * @param skirt width of transition band between cutoff and stopband
     * @param sample_rate sample rate for the input stream. 
     * @param num_taps number of taps in the filter.
     * @param buffer_size size of the input buffer when apply is called
     * @param gain relative magnitude of input to output in the passband
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN
     */
    Filter(float low_cutoff, float high_cutoff, float skirt,
	   float sample_rate,
	   unsigned int num_taps,
	   unsigned int buffer_size, 
	   float gain = 1.0, 
	   WindowChoice window = HANN);    

    /**
     * @brief Alternate constructore where we just get the H proto
     *
     * @param H a prototype frequency domain image of the filter
     * @param buffer_size size of the input buffer when apply is called
     * @param gain relative magnitude of input to output in the passband
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN
     */
    Filter(std::vector<std::complex<float>> & H, 
	   unsigned int buffer_size, 
	   float gain = 1.0,
	   WindowChoice window = HANN);
    
    /// run the filter on a complex input stream
    /// @param in_buf the input buffer I/Q samples (complex)
    /// @param out_buf the output buffer I/Q samples (complex)
    /// @param in_out_mode input or output can be time samples or frequency (FFT format) samples
    /// @return the length of the input buffer
    unsigned int apply(std::vector<std::complex<float>> & in_buf, 
		       std::vector<std::complex<float>> & out_buf, 
		       InOutMode in_out_mode = InOutMode(true,true));

    /// run the filter on a real input stream
    /// @param in_buf the input buffer samples
    /// @param out_buf the output buffer samples (this can overlap the in_buf vector)
    /// @param in_out_mode ignored -- input and output must be time-domain samples.
    /// @return the length of the input buffer
    ///
    /// Throws Filter::BadRealFilter if the original filter spec was not "real"
    unsigned int apply(std::vector<float> & in_buf, 
		       std::vector<float> & out_buf, 
		       InOutMode in_out_mode = InOutMode(true,true));

    /**
     * @brief create a hamming window to make the filter nice.
     * @param w a vector that will be multiplied by the filter image
     */
    static void hammingWindow(std::vector<float> & w);

    /**
     * @brief create a hann window to make the filter nice.
     * @param w a vector that will be multiplied by the filter image
     */
    static void hannWindow(std::vector<float> & w);

    /**
     * @brief create a blackman window to make the filter nice.
     * @param w a vector that will be multiplied by the filter image
     */
    static void blackmanWindow(std::vector<float> & w);        

    
    /**
     * @brief Return the lowest and highest corner frequency for this filter. 
     */
    std::pair<float, float> getFilterEdges();

    /**
     * @brief how long must an output buffer be?
     * 
     * @param in_size the size of an input buffer to this filter
     * @return the same as in_size
     */
    unsigned int outLenRequired(unsigned int in_size) { return in_size; }

    /**
     * @brief Create shared pointer to a filter from a filter spec for a general filter
     *
     * @param filter_spec object of class FilterSpec identifying corner frequencies and amplitudes
     * @param buffer_size the impulse response and frequency image will be this long
     * @param gain passband gain (max gain) through filter
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN
     * @return shared pointer to a Filter     
     */
    static std::shared_ptr<Filter> make(FilterSpec & filter_spec, 
					unsigned int buffer_size, 
					float gain = 1.0, 
					WindowChoice window = HANN);        


    /**
     * @brief Shared pointer: Alternate constructor, for very simple filters
     *
     * @param low_cutoff lower edge of the filter
     * @param high_cutoff upper edge of the filter
     * @param skirt width of transition band between cutoff and stopband
     * @param sample_rate sample rate for the input stream. 
     * @param num_taps number of taps in the filter.
     * @param buffer_size size of the input buffer when apply is called
     * @param gain relative magnitude of input to output in the passband
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN
     * @return shared pointer to a Filter     
     */
    static std::shared_ptr<Filter> make(float low_cutoff, float high_cutoff, float skirt,
					float sample_rate,
					unsigned int num_taps,
					unsigned int buffer_size, 
					float gain = 1.0, 
					WindowChoice window = HANN);    

    /**
     * @brief Shared pointer: Alternate constructore where we just get the H proto
     *
     * @param H a prototype frequency domain image of the filter
     * @param buffer_size size of the input buffer when apply is called
     * @param gain relative magnitude of input to output in the passband
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN
     * @return shared pointer to a Filter
     */
    static std::shared_ptr<Filter> make(std::vector<std::complex<float>> & H, 
					unsigned int buffer_size, 
					float gain = 1.0,
					WindowChoice window = HANN);
    
  private:
    /** @brief  Build the filter from a filter spec for a bandpass filter -- common method
     * for all forms of Filter constructors. 
     * 
     * @param filter_spec object of class FilterSpec identifying corner frequencies and amplitudes
     * @param buffer_size the impulse response and frequency image will be this long
     * @param gain passband gain (max gain) through filter
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN 
     * 
     */
    void makeFilter(FilterSpec & filter_spec, 
		    unsigned int buffer_size, 
		    float gain = 1.0,
		    WindowChoice window = HAMMING); 		    

    /** @brief  Build the filter from a prototype
     *
     * @param Hproto frequency domain filter image
     * @param buffer_size the impulse response and frequency image will be this long
     * @param gain passband gain (max gain) through filter
     * @param window filter window choice - we're using the window filter synthesis method. Defaults to HANN 
     * 
     */
    void makeFilter(std::vector<std::complex<float>> Hproto, 
		    unsigned int buffer_size,
		    float gain = 1.0, 		    
		    WindowChoice window = HAMMING); 


    
    // this is the FFT image of the filter
    std::vector<std::complex<float>> H;  ///< FFT image of the filter

    std::vector<std::complex<float>> h; ///< impulse response of the filter

    ///< We need an FFT widget for the input/output transforms
    std::unique_ptr<FFT> fft; 
    
    ///< and we need a temporary vector for the frequency domain product
    std::vector<std::complex<float>> temp_buf;

    ///< and a two more vectors if we're doing a real valued filter
    std::vector<std::complex<float>> temp_in_buf;
    std::vector<std::complex<float>> temp_out_buf;        

    /// This is the size of the block (number of samples) that we'll operate on. 
    unsigned int buffer_size; 
    float sample_rate;  ///< does this really need documentation? 
  };
}

