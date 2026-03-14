#include "../include/Format.hxx"
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <random>
/*
BSD 2-Clause License

Copyright (c) 2026 Matt Reilly - kb1vc
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

bool check(const std::string & cor, SoDa::Format & fmt) {
  auto tst = fmt.str();
  if(tst != cor) {
    std::cerr << "Got \"" << tst << "\" expected \"" << cor << "\"\n";
    return false; 
  }
  return true; 
}

int main(int argc, char * argv[]) {
  SoDa::Format fmt1("This is a test %0 %1 %2");

  SoDa::Format fmt2(""), fmt3("");

  bool res = true;
  
  fmt2 = fmt1; 
  res = res && check("This is a test 1 %1 %2", fmt1.addI(1));
  res = res && check("This is a test 1 2 %2", fmt1.addI(2));  
  res = res && check("This is a test 3 %1 %2", fmt2.addI(3));
  fmt3 = fmt1;
  fmt3.reset();
  res = res && check("This is a test %0 %1 %2", fmt3);
  res = res && check("This is a test 4 %1 %2", fmt3.addI(4));
  res = res && check("This is a test 1 2 5", fmt1.addI(5));

  if(res) {
    std::cout << "PASS\n";
  }
  else {
    std::cout << "FAIL\n";    
  }
}
