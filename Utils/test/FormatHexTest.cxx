#include "../include/Format.hxx"
#include <string>
#include <iostream>
#include <map>

/*
BSD 2-Clause License

Copyright (c) 2021,2025 Matt Reilly - kb1vc
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

bool testFmt(unsigned long v, int w, char fmt_type, const std::string & pat, char sep) {
  std::string res = SoDa::Format("%0").addU(v, fmt_type, w, sep).str(); 
  
  if (res != pat) {
    std::cerr << SoDa::Format("Bad match: v = %0, w = %1, res = [%2] pattern = [%3] type %4\n")
      .addU(v, 'X')
      .addI(w)
      .addS(res)
      .addS(pat)
      .addC(fmt_type);
    return false; 
  }
  
  return true; 
}
int main(int argc, char * argv[]) {

  
  std::map<std::string, std::pair<unsigned long, int>> hex_check_map = 
    { { "0x0", { 0, 0 } }, 
      { "0x000", { 0, 3} },
      { "0x0000", { 0, 4} },       
      { "0x100", { 0x100, 0} },
      { "0x0100", { 0x100, 4 } },
      { "0x00000100", { 0x100, 8 } },
      { "0x0000000000000100", { 0x100, 16 } }            
    };

  std::map<std::string, std::pair<unsigned long, int>> dec_check_map = 
    { { "0", { 0, 0 } }, 
      { "  0", { 0, 3} }, 
      { "100", { 100, 0} },
      { " 100", { 100, 4 } },
      { "     100", { 100, 8 } },
      { "             100", { 100, 16 } }            
    };
  std::map<std::string, std::pair<unsigned long, int>> oct_check_map = 
    { { "0", { 0, 0 } }, 
      { "000", { 0, 3} }, 
      { "0137", { 0137, 0} },
      { "0137", { 0137, 4 } },
      { "00000137", { 0137, 8 } },
      { "0000000000000137", { 0137, 16 } }            
    };
  
  
  std::map<std::string, std::pair<unsigned long, int>> under_check_map = 
    { { "0x0", { 0, 0 } }, 
      { "0x000", { 0, 3} }, 
      { "0x100", { 0x100, 0} },
      { "0x0100", { 0x100, 4 } },
      { "0x0000_0100", { 0x100, 8 } },
      { "0x0000_0000_0000_0100", { 0x100, 16 } }            
    };
  
  bool all_ok = true; 
  for(auto e : hex_check_map) {
    all_ok = all_ok && testFmt(e.second.first, e.second.second, 'X', e.first, '\000');
  }
  for(auto e : under_check_map) {
    all_ok = all_ok && testFmt(e.second.first, e.second.second, 'X', e.first, '_');
  }
  for(auto e : dec_check_map) {
    all_ok = all_ok && testFmt(e.second.first, e.second.second, '\000', e.first, '\000');
  }
  for(auto e : oct_check_map) {
    all_ok = all_ok && testFmt(e.second.first, e.second.second, 'o', e.first, '\000');
  }
    
  if(all_ok) exit(0);
  else exit(-1);
}
