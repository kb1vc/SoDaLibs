#pragma once
#include <complex>
#include <vector>
#include "Filter.hxx"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <SoDa/Format.hxx>
#include <iostream>
#include <fstream>

namespace SoDa {
  
  class ReSampler_BadBufferSize : public std::runtime_error {
  public:
    ReSampler_BadBufferSize(int in_blen, int interp, int dec) :
      std::runtime_error(SoDa::Format("SoDa::ReSampler Buffer Length %0 can't be rationally resampled as %1/%2\n")
			 .addI(in_blen)
			 .addI(interp)
			 .addI(dec)
			 .str()) {
    }
  };

  class ReSampler_MismatchBufferSize : std::runtime_error {
  public:
    ReSampler_MismatchBufferSize(const std::string dir, 
				 int in_blen, 
				 int exp_in_blen) :
      std::runtime_error(SoDa::Format("SoDa::ReSampler %0put Buffer Length %1 should have been %2\n")
			 .addS(dir)
			 .addI(in_blen)
			 .addI(exp_in_blen)
			 .str()) {
    }
  };
  
  template <typename T>
  class ReSampler {
  public:
    
    /**
     * @brief Constructor -- A Rational Resampler in the frequency domain
     * 
     * This builds on the overlap-and-save SoDa::Filter
     * 
     * @param in_buffer_len length of input buffer to be resampled
     * @param interpolate interpolation (upsampling) factor
     * @param decimate decimation (downsampling) factor
     * @param transition_width width of skirt between output low-pass freq
     * and output nyquist rate
     *
     * This constructor will throw a "SoDa::Resampler_BadRate" exception
     * if the up/down ratio can't be implemented in the specified buffer
     * lengths. 
     *
     * ## The Algorithm
     *
     * This is a fairly straightforward filter-based implementation of a 
     * rational resampler. It takes place in two stages: Upsampling, 
     * followed by Dowsampling. Most of the heavy lifting is in the frequency
     * domain.  Interpolate by I, decimate by D
     * 
     * Because the resampling is continuous *and* we want to do the heavy
     * lifting in the frequency domain, we'll use an overlap-and-save 
     * filter implementation on the upsampled input: 
     * 
     * 1. x'[nI] = x[n]
     * 2. filter x' with cutoff +/- Fc
     * 3. y[n] = y'[nD]
     *
     * The BPF corner frequency Fc is relative to the new upsampled
     * rate Fs' , so if I > D  Fc =  Fs / (2I)  
     * if I < D Fc = Fs / (2D)    
     * 
     * ## The implementation
     * 
     * We'll use an overlap-and-save method, building on the components of the
     * SoDa::Filter class. The LPF filter H[] will be created in the Filter. Its
     * bandwidth will be set by the Interpolate and Decimate factors so that the
     * aliasing in both steps is addressed.  The cutoff frequency needs to be
     * low enough to provide the interpolation smoothing for the upsampling and
     * the alias elimination for the downsampling. 
     */
    ReSampler(int in_buffer_len, 
	      int interpolate,
	      int decimate,
	      float transition_width) : 
      in_buffer_len(in_buffer_len), interpolate(interpolate), decimate(decimate)
    {
      transition_width = 0.1;
      int out_buffer_len = (in_buffer_len * interpolate) / decimate;

      if(((out_buffer_len * decimate) / interpolate) != in_buffer_len) {
	// then we screwed up and the ratio won't work. 
	// yes, this is overly restrictive, but why buy agony wholesale?
	throw SoDa::ReSampler_BadBufferSize(in_buffer_len, interpolate, decimate); 
      }

      float sample_rate = 1;
      float fq1 = 0.4 / ((float) interpolate); 
      float fq2 = 0.4 / ((float) decimate);

      float freq_corner = (interpolate > decimate) ? fq1 : fq2; 

      int interpolate_buf_len = in_buffer_len * interpolate;
      
      lpf.initFilter(SoDa::FilterType::BP, 41, 
		     sample_rate, -freq_corner, freq_corner,
		     0.1 * freq_corner,
		     50, 
		     interpolate_buf_len);


      interp_buffer.resize(interpolate_buf_len); 

      // zero out the interpolate buffer.
      std::fill(interp_buffer.begin(), interp_buffer.end(), std::complex<T>(0.0,0.0));

      debug_count = 0; 
    }

    /**
     * @brief Destructor
     * 
     */
    ~ReSampler() {
    }
    
    /** 
     * @brief Apply the Resampler to a buffer stream
     * 
     * @param out The output of the filter. 
     * @param in The input to the filter. 
     */
    void applyCont(std::vector<std::complex<T>> & out, std::vector<std::complex<T>> & in) {

      std::cerr << SoDa::Format("stuffing interp buffer  in.size %0  interp.size %1  out.size %2\n")
	.addI(in.size())
	.addI(interp_buffer.size())
	.addI(out.size());
      
      // zero stuff -- note the interp_buffer is pre-zeroed.
      float scale = interpolate; 
      for(int i = 0; i < in.size(); i++) {
	interp_buffer[i * interpolate] = in[i] * scale; 
      }
      std::cerr << "About to apply filter\n";
      // do the anti-aliasing filter and decimate on the way out. 
      lpf.applyCont(out, interp_buffer, decimate);

      debug_count++; 
      std::cerr << "Done\n";
      return; 
    }

      

  protected:

    void dump(const std::string & fn, std::vector<std::complex<T>> vec) {
      std::ofstream out(fn);

      for(auto & v : vec) {
	T mag = v.real() * v.real() + v.imag() * v.imag();
	out << v.real() << " " << v.imag() << " " << mag << "\n";
      }

      out.close();
    }
    
    ///@{
    int debug_count;
    
    int interpolate, decimate; 
    int in_buffer_len; 

    std::vector<std::complex<T>> interp_buffer;
    
    SoDa::Filter<T> lpf; // lowpass filter for interp/decimation
    
    ///@}
  };

}
