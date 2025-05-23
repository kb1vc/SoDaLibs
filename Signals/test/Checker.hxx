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

#include <vector>
#include <complex>
#include <functional>
#include "../include/Periodogram.hxx"
#include "../include/NCO.hxx"
#include "../include/Utilities.hxx"

namespace SoDa {
  typedef std::vector<std::complex<float>> CVec; 
  class Checker {
  public:
    Checker(double output_sample_rate, uint32_t filter_length, 
	    double ripple_limit_db, double stopband_atten, 
	    double permissible_phase_error,
	    uint32_t buffer_length,
	    uint32_t freq_steps = 1024);

    Checker(double output_sample_rate, uint32_t filter_length, 
	    double ripple_limit_db, double stopband_atten, 
	    double permissible_phase_error,
	    uint32_t input_buffer_length,
	    uint32_t output_buffer_length, 
	    double input_sample_rate,
	    uint32_t freq_steps = 1024);

    void initialize(double output_sample_rate, uint32_t filter_length, 
	    double ripple_limit_db, double stopband_atten, 
	    double permissible_phase_error,
	    uint32_t input_buffer_length,
	    uint32_t output_buffer_length, 
	    double input_sample_rate,
		    uint32_t freq_steps);

		    
    enum CheckRegion { STOP_BAND, PASS_BAND, TRANSITION_BAND };

    void checkResponse(uint32_t freq_step,
		       std::function<CheckRegion(double)> freqRegion, 
		       std::function<void(std::vector<std::complex<float>> &,
					  std::vector<std::complex<float>> &)> filt);
        
    bool testPassed() { return test_passed; }

    uint32_t getNumFreqSteps();
    
    double getFreq(uint32_t freq_step); 


  protected:
    double phase(double freq);
    
    uint32_t filter_length;
    double permissible_phase_error;
    double target_phase_shift;
    double ripple_limit_dB;
    double stopband_gain_dB;
    double threshold_min, threshold_max;
    uint32_t input_buffer_length, output_buffer_length; 
    
    CheckRegion check_region;    

    double output_sample_rate, input_sample_rate, high_sample_rate;
    
    bool test_passed; 
    NCO ref_nco;
    NCO input_nco; 

    std::vector<double> frequencies;
    std::vector<std::vector<std::complex<float>>> first_output_oscillators, second_output_oscillators;
    std::vector<std::vector<std::complex<float>>> first_input_oscillators, second_input_oscillators;     
  };
}
