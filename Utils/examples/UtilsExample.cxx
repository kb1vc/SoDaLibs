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

/**
 * @file UtilsExample.cxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 12, 2021
 */


#include <iostream>
#include <SoDa/Utils.hxx>
#include <string>
#include <list>

int main(int argc, char * argv[])
{
  // squash spaces out of a string
  std::string test_in = "This is a  twospace   threespace test   ";
  std::string test_out = SoDa::squashSpaces(test_in);
  std::cout << "Input string [" << test_in << "]\n";
  std::cout << "Output string [" << test_out << "]\n";
  
  // split a string into tokens
  std::list<std::string> tkns = SoDa::split(test_in, " ,");
  std::cout << "Split this one [" << test_in << "]\n";
  for(auto e : tkns) {
    std::cout << "[" << e << "]\n";
  }
  
  std::string test2_in = "This, is a,test,one,two,,three ";
  std::list<std::string> tkns2 = SoDa::split(test2_in, " ,", true);
  std::cout << "Split this one [" << test2_in << "]\n";  
  for(auto e : tkns2) {
    std::cout << "[" << e << "]\n";    
  }
  
}
