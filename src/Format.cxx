#include "Format.hxx"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <ctype.h>
#include <regex>
#include <algorithm>

/*
BSD 2-Clause License

Copyright (c) 2020, Matt Reilly - kb1vc
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


namespace SoDa {
  char Format::separator = '.';

  Format::FmtStringSeg::FmtStringSeg(const std::string & str) {
    seg_type = LITERAL;
    val = str; 
  }
  
  Format::FmtStringSeg::FmtStringSeg(unsigned int _idx) {
    seg_type = FMT_VAL;
    idx = _idx;
  }

  Format::Format(const std::string & fmt_string) {
    // save the format string
    orig_fmt_string = fmt_string; 
    // build the format list
    initialScan(fmt_string);
    // we're looking for the first argument
    cur_arg_number = 0;
  }


  void Format::initialScan(const std::string & fmt_string) {
    format_string_segments.clear();    
    // scan the format string
    enum ScanState { NORM, SAW_PC, ACC_FLDNUM };
    ScanState s_state = NORM;

    std::string cur_str;
    unsigned int cur_fldnum = 0; 
    for(auto c : fmt_string) {
      switch (s_state) {
      case NORM:
	if(c == '%') {
	  // we might be looking at a field specifier
	  s_state = SAW_PC;
	}
	else {
	  cur_str.push_back(c); 
	}
	break; 
      case SAW_PC:
	// are we looking at a field number?
	if(isdigit(c)) {
	  // we should push the current string
	  if(!cur_str.empty()) {
	    format_string_segments.push_back(FmtStringSeg(cur_str));
	    cur_str.clear();
	  }

	  // and indicate that we're scanning for the rest of the field number
	  s_state = ACC_FLDNUM; 
	  cur_fldnum = (unsigned int) (c - '0');
	}
	else {
	  // push a % and the current char onto the current string
	  cur_str.push_back('%');
	  // if the input was %%, we just send one %, otherwise, include the next char
	  if(c != '%') cur_str.push_back(c);
	  // and now we're normal
	  s_state = NORM;
	}
	break; 
      case ACC_FLDNUM:
	if(isdigit(c)) {
	  cur_fldnum = cur_fldnum * 10 + ((unsigned int) (c - '0'));
	}
	else {
	  // we scanned an fld, now push the fmt specifier
	  format_string_segments.push_back(FmtStringSeg(cur_fldnum));
	  // set the state
	  s_state = NORM;
	  // clear the current string
	  cur_str.clear();
	  // and save the character
	  cur_str.push_back(c); 
	}
	break; 
      }
    }

    switch (s_state) {
    case SAW_PC:
      // we got to end-of-string with a % at the end.
      cur_str.push_back('%');
      // fall through. 
    case NORM:
      if(!cur_str.empty()) {
	format_string_segments.push_back(FmtStringSeg(cur_str));
      }
      break;
    case ACC_FLDNUM:
      format_string_segments.push_back(FmtStringSeg(cur_fldnum));
      break; 
    }
  }

  Format & Format::addI(int v, unsigned int w, char sep) {
    std::stringstream ss;
    if(sep == '\000') {
      if(w != 0) {
	ss << std::setw(w); 
      }
      ss << v;

    }
    else {
      // print with comma separators
      std::stringstream pre_ss;
      pre_ss << v;
      std::string vstr(pre_ss.str());
      std::reverse(vstr.begin(), vstr.end());
      
      std::string newstr;
      int i = 0; 
      for(auto c : vstr) {
	if(i == 3) {
	  newstr = sep + newstr;
	  i = 0; 
	}
	newstr = c + newstr;
	i++;
      }

      if(w != 0) {
	ss << std::setw(w);
      }
      ss << newstr;
    }
    insertField(ss.str());     
    return * this; 
  }

  Format & Format::addU(unsigned long v, char fmt, unsigned int w,
			char sep,
			unsigned int group_count)
{
    std::stringstream ss;
    std::stringstream pre_ss;

    auto radhint = std::dec;
    std::string prefix("");
    switch(fmt) {
    case 'x':
    case 'X':       
      radhint = std::hex;
      prefix = "0x";
      break;
    case 'o': radhint = std::oct;
      prefix = "0";
      break;
    default:
      radhint = std::dec;
      break; 
    }

    if(w != 0) {
      unsigned int nw = (w > prefix.size()) ? (w - prefix.size()) : w;
      ss << std::setw(nw); 
    }

    if(sep == '\000') {
      if(fmt == 'X') pre_ss << std::uppercase;
      pre_ss << radhint <<  v;
      ss << (prefix + pre_ss.str());      
    }
    else {
      std::stringstream new_ss;
      if(fmt == 'X') new_ss << std::uppercase;      
      new_ss << radhint << v; 
      std::string vstr(new_ss.str());
      int i = 0;
      std::reverse(vstr.begin(), vstr.end());
      std::string newstr; 

      for(char c : vstr) {
	if(i == group_count) {
	  newstr = sep + newstr; 
	  i = 0; 
	}
	newstr = c + newstr;
	i++; 
      }
      newstr = prefix + newstr; 
      ss << newstr;
    }

    
    insertField(ss.str());     
    return *this;     
  }
  
  int log1k(double v, double & v_norm, int sig_digs) {
    int ret = 0;
    v_norm = v; 
    // round it first
    double remul = 1.0; 
    while(v_norm >= 10.0) {
      v_norm = v_norm / 10.0;
      remul = remul * 10.0;
    }
    while(v_norm < 1.0) {
      v_norm = v_norm * 10.0;
      remul = remul / 10.0;
    }
    // now create the round incr
    double ri = 5;    
    for(int s = 0; s < sig_digs; s++) {
      ri = ri / 10.0;
    }
    // std::cerr << "ri = " << ri << " sig_digs " << sig_digs << "\n";
    // now do the round-up add
    v_norm += ri;
    //std::cerr << "Now v_norm = " << v_norm << "\n";

    // now restore v_norm to the original magnitude more or less
    v_norm = v_norm * remul;

    // ok, now take the log1k
    if(v_norm < 1.0) {
      while(v_norm < 1.0) {
	ret -= 3; 
	v_norm = v_norm * 1000.0; 
      }
    }
    else {
      while(v_norm > 1000.0) {
	ret += 3; 
	v_norm = v_norm / 1000.0; 
      }
    }
    return ret; 
  }

  int getIntPartWidth(double v) {
    int ret = 0; 
    while(v > 1.0) {
      v = v / 10.0; 
      ret++; 
    }
    return ret; 
  }
  
  void fractionate(double v, unsigned int significant_digits, 
		   int & int_part, int & frac_part,
		   int & int_wid, int & frac_wid) {
    // v is in the range 1 to 1000 - epsilon. 
    // How many digits did it take up?
    int_wid = getIntPartWidth(v); 

    // get fractional and int parts.
    double dfrac_part, dint_part;
    dfrac_part = modf(v, &dint_part); 
    
    //std::cerr << "dint_part " << dint_part << " dfrac_part " << dfrac_part 
    //	      << "\n";
    int_part = (int) dint_part;
    //std::cerr << "int_part " << int_part << "\n";
    
    // now we need to zap off any integer parts
    // if we don't have enough significant digits.
    int modval = 1;
    int iw = int_wid;
    while(iw > significant_digits) {
      modval = modval * 10; 
      iw--;
    }

    //std::cerr << " int_part " << int_part << " vs modval " << modval;
    int_part = int_part - (int_part % modval);      
    //std::cerr << " gets us " << int_part << "\n";
    
    //std::cerr << "dfrac_part " << dfrac_part  << "\n";
    // now trim the fractional part. 
    frac_wid = 0; frac_part = 0;
    int sd_left = significant_digits - iw;
    //std::cerr << "sd_left = " << sd_left << "\n";
    while(sd_left > 0) {
      dfrac_part = dfrac_part * 10.0;
      //std::cerr << "New dfrac_part " << dfrac_part << " floor(dfp) " << floor(dfrac_part) 
      //		<< " frac_part " << frac_part << "\n";
      frac_part = 10 * frac_part + floor(dfrac_part);
      double junk;
      dfrac_part = modf(dfrac_part, &junk);
      //std::cerr << "new frac_part " << frac_part 
      //		<< " dfrac_part " << dfrac_part
      //	<< " trunc(df_p) " << trunc(dfrac_part)
      //	<< "\n";
      frac_wid++;
      sd_left--;
    }
  }
  
  Format & Format::addF(double v, char fmt, unsigned int width, unsigned int significant_digits) {
    std::stringstream ss;    
    //std::cerr << "v = " << v << " width = " << width 
    //	      << " significant_digits = " << significant_digits
    //	      << "\n";
    ss << std::left << std::setfill(' ');

    if(width == 0) {
      width = significant_digits + 4;
    }
    switch (fmt) {
    case 'f':
      // fixed floating point format
      if(width) ss << std::setw(width); 
      ss << std::fixed << std::setprecision(significant_digits) << v;  
      break; 
    case 's':
      // scientific (who cares what the exponent is? format)
      if(width) ss << std::setw(width);       
      ss << std::scientific << std::setprecision(significant_digits) << v;
      break; 
    case 'g':
      // general (who cares what the exponent is format, or how this looks)
      ss.unsetf(std::ios::fixed | std::ios::scientific);
      if(width) ss << std::setw(width);             
      ss << std::setprecision(significant_digits) << v;
      break; 
    case 'e':
      // now this is a tough one.
      // the object is to print this number as xxx.fffeN where N is a multiple of 3.
      // (engineering notation).  It is beyond me how C and C++ have continued to print
      // crap floating point formats that are only suited to imperial units (black and
      // white TV) or astronomers.  It is 2021 -- time to use the metric system, even
      // in backwaters that still cling to the 16th century.

      // it is possible that the value is 0.  if so, just jump skip the rest of this.
      if(v == 0.0) {
	//std::cerr << "Got zero " << v << "\n";
	ss << "0" << separator << std::left 
	   << std::setw(significant_digits - 1) << std::setfill('0') << 0 << "e0";
      }
      else {
	// get the abs val
	//std::cerr << "Got v = " << v << "\n";
	double av = fabs(v);
	// now get the log base 1000
	// That's probably going to be our
	// exponent. (Unless the fractional part
	// rolls over.) We also normalize to
	// a value in the range 1 to 1000
	double av_norm; 
	int exp_val = log1k(av, av_norm, significant_digits);
	//std::cerr << "av_norm " << av_norm << " exp_val " << exp_val << "\n";
	// now do the fraction and rounding
	int frac_part, int_part, int_wid, frac_wid;
	fractionate(av_norm, significant_digits,
		    int_part, frac_part, 
		    int_wid, frac_wid);
	//std::cerr << "int_part " << int_part << " frac_part " << frac_part
	//<< " int_wid " << int_wid << " frac_wid " << frac_wid
	//	  << "\n";
	// now print the sign
	ss << ((v < 0) ? '-' : ' ');
	// and the integer part
	ss << std::setw(int_wid) << int_part;
	// if we have no significant fraction digits,
	// leave them off.
	if(frac_wid > 0) {
	  ss << '.' << std::setw(frac_wid) << std::right 
	     << std::setfill('0') << frac_part;
	}
	// now the exponent

	ss << std::setw(1) << 'e'
	   << std::setfill(' ') << std::left; 
	if(exp_val < 0) {
	  ss << '-' << std::setw(2) << -exp_val;
	}
	else {
	  ss << '+'  << std::setw(2) << exp_val;	  
	}
	// now fill the rest of the field;
	int fill_wid = width - (int_wid + frac_wid + 4); 
	while(fill_wid > 0) {
	  ss << ' ';
	  fill_wid--;
	}
      }
    }

    insertField(ss.str());
    return *this;     
  }

  Format & Format::addS(const std::string & v, int width) {
    std::string fstr;
    if(width < 0)  {
      for(auto c : v) {
	fstr.push_back(c);
      }
      int spaces = (- width) - v.size();
      for(int i = 0; i < spaces; i++) {
	fstr.push_back(' ');
      }
      insertField(fstr);      
    }
    else if (width > 0) {
      int spaces = width - v.size();
      for(int i = 0; i < spaces; i++) {
	fstr.push_back(' ');
      }
      for(auto c : v) {
	fstr.push_back(c);
      }
      insertField(fstr);
    }
    else {
      insertField(v);
    }
    return *this;     
  }

  Format & Format::addC(char c) {
    std::stringstream ss;
    ss << c; 
    insertField(ss.str());
    return *this;
  }

  void Format::insertField(const std::string & s) {
    for (auto & fld : format_string_segments) {
      if((fld.seg_type == FmtStringSeg::FMT_VAL) && (fld.idx == cur_arg_number)) {
	fld.val = s; 
	fld.seg_type = FmtStringSeg::LITERAL; 
      }
    }

    cur_arg_number++;
  }

  
  Format & Format::reset() {
    initialScan(orig_fmt_string);
    cur_arg_number = 0; 
    return *this;
  }
  

  std::string Format::str(bool check_for_filled_out) const {
    std::string ret_string;
    // assemble the string, warts and all
    int unfilled_count = 0; 
    for(auto & fld : format_string_segments) {
      if(fld.seg_type == FmtStringSeg::LITERAL) {
	ret_string = ret_string + fld.val;
      }
      else if(fld.seg_type == FmtStringSeg::FMT_VAL) {
	std::stringstream ss;
	ss << '%' << fld.idx;
	ret_string = ret_string + ss.str();
	unfilled_count++; 
      }
    }

    if(check_for_filled_out && (unfilled_count != 0)) {
      throw BadFormat("Unfilled argument string [" + ret_string + "]", *this);
    }

    return ret_string; 
  }

  double Format::roundToSigDigs(double v, int sig_digits) {
    double ret = v;
    double significance_threshold = pow(10.0, sig_digits) - 0.5;
    double shift_correction = 1.0;

    if(ret > significance_threshold) {
      // divide down until we're ready to round
      while(ret > significance_threshold) {
	ret = ret * 0.1;
	shift_correction *= 10.0;
      }
    }
    else if(ret < significance_threshold) {
      while(ret < significance_threshold) {
	ret = ret * 10.0;
	shift_correction *= 0.1;
      }
      ret = ret * 0.1;
      shift_correction *= 10.0;
    }

    // now calculate the round increment.
    double fpart = ret - floor(ret);
    if(fpart >= 0.5) ret = ret + 0.5;
      
    ret = trunc(ret);

    return ret * shift_correction;
  }
}



std::ostream & operator<<(std::ostream & os, const SoDa::Format & f) {
  os << f.str(false);
  return os; 
}
