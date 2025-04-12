#include "../include/Utils.hxx"
#include <iostream>
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



int main() {
  std::string comma_test = "this,that,the other thing";
  std::vector<std::string> comma_ref = {"this", "that", "the other thing" };

  auto sv = SoDa::splitVec(comma_test, ",");
  auto sl = SoDa::split(comma_test, ",");
  
  bool pass = true; 

  for(int i = 0; i < comma_ref.size(); i++) {
    if(sv[i] != comma_ref[i]) {
      std::cerr << "splitVec [" << comma_test << "] got [" << sv[i] << "] for arg " << i << " should have been [" << comma_ref[i] << "]\n";
      pass = false;
    }
  }
  int i = 0;
  for(auto s : sl) {
    if(s != comma_ref[i]) {
      std::cerr << "Split [" << comma_test << "] got [" << s << "] for arg " << i << " should have been [" << comma_ref[i] << "]\n";
      pass = false;
    }
    i++; 
  }

  std::string blank_bug_test = "";
  std::string one_space_test = " ";
  // split should return an empty list for empty strings
  auto bbt = SoDa::split(blank_bug_test, ",");
  if(bbt.size() != 0) {
    std::cerr << "split of empty string yielded vector of non-zero length.\n";
    pass = false;
  }
  auto bbtv = SoDa::splitVec(blank_bug_test, ",");  
  if(bbtv.size() != 0) {
    std::cerr << "splitVec of empty string yielded vector of non-zero length.\n";
    pass = false;
  }

  auto ost = SoDa::split(one_space_test, ",");
  if(ost.size() != 0) {
    std::cerr << "split of string with one space yielded vector of non-zero length.\n";
    pass = false;
  }
  auto ostv = SoDa::splitVec(one_space_test, ",");  
  if(ostv.size() != 0) {
    std::cerr << "splitVec of string with one space yielded vector of non-zero length.\n";
    pass = false;
  }
  
  
  if(pass) {
    std::cerr << "PASS\n";
  }
  else {
    std::cerr << "FAIL\n";
  }
}
