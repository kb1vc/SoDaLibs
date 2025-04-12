#include "Utils.hxx"
#include <iostream>

/*
BSD 2-Clause License

Copyright (c) 2021, Matt Reilly - kb1vc
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

/**
 * @file Split.cxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 12, 2021
 */

#include <string>

namespace SoDa {

  std::string squashSpaces(const std::string & str) {
    std::string ret = str;
    size_t pos = 0;

    if(ret.size() == 0) {
      return ret;
    }

    // trim the string. 
    while((ret.size() != 0) && (ret.front() == ' ')) {
      ret.erase(0, 1);
    }
    if(ret.size() == 0) {
      return ret;
    }

    while((ret.size() != 0) && (ret.back() == ' ')) ret.pop_back();
    if(ret.size() == 0)  {
      return ret;
    }

    while((pos = ret.find(' ', pos)) != std::string::npos) {
      if((pos < ret.length() - 1) && (ret[pos+1] == ' ')) {
	ret.erase(pos, 1);
      }
      else {
	pos = pos + 1;
      }
    }
    
    return ret; 
  }
  
  std::list<std::string> split(const std::string & str, const std::string delims, bool no_empty) {
    std::list<std::string> ret;
    std::string wrk = squashSpaces(str);

    size_t pos = 0; 
    size_t old_pos = 0; 

    // scan for delims
    while((pos = wrk.find_first_of(delims, old_pos)) != std::string::npos) {
      std::string tkn = wrk.substr(old_pos, pos - old_pos);
      if(no_empty && (tkn.length() == 0)) {
	// do nothing.  -- yup, I know.  But would you write it more clearly?
      }
      else {
	ret.push_back(tkn); 
      }

      old_pos = pos + 1; // skip over the delmiter.
    }

    if(old_pos < wrk.length()) {
      ret.push_back(wrk.substr(old_pos));
    }

    return ret; 
  }

  std::vector<std::string> splitVec(const std::string & str, const std::string delims, bool no_empty) {
    std::vector<std::string> ret; 
    
    auto tres = split(str, delims, no_empty);
    
    for(auto s : tres) {
      ret.push_back(s);
    }

    return ret; 
  }
  
}
