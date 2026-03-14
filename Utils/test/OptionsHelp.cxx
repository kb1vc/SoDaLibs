#include "../include/Format.hxx"
#include "../include/Options.hxx"
#include <iostream>
#include <cmath>
#include <type_traits>
#include <set>
#include <list>
#include <functional>
#include <cstring>
#include <memory>
#include <typeinfo>


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

#include <stdlib.h>

// expected strings
std::string bad_option_text = R"(Command option [[--h]] is unknown.
)";
std::string expected_text = R"(
usage:    OptionsHelp [options]

    Where options include:

   --float                -f     Float (default: 1)
   --help                 -h     Print this message (default: False)
   --int                  -i     Integer (default: 100)
   --present              -p     Present (default: False)
   --string               -s     String (default: test)
)";


char * string2char(const std::string & str) {
  char * ret = (char *) malloc(str.size() + 1);
  strncpy(ret, str.c_str(), str.size());
  return ret;
}
bool testCompare(const std::string & expected, 
		 const std::string & got,
		 const std::string & help_switch) {
  if(expected != got) {
    std::cerr << " with help option = \"" << help_switch << "\"\n";
    std::cerr << "Expected:\n================================================================\n"
	      << expected
	      << "================================================================\n"		
	      << "Got:\n================================================================\n"
	      << got
	      << "================================================================\n"	
	      << "\n";

    for(int i = 0; i < std::min(expected.size(), got.size()); i++) {
      if(expected[i] == got[i]) {
	std::cerr << expected[i];
      }
      else {
	std::cerr << "\nmiscompare: expected '" << expected[i] << "' got '" << got[i] << "\n";
	break; 
      }
    }
    return false;
  }
  return true; 
}

bool doTest(const std::string help_switch, std::string & got_text) {
  SoDa::Options cmd;
  int ival;
  float fval;
  bool pval;
  bool hval; 
  std::string sval;

  cmd.add<int>(&ival, "int", 'i', 100, "Integer");
  cmd.add<float>(&fval, "float", 'f', 1.0, "Float");
  cmd.add<std::string>(&sval, "string", 's', "test", "String");
  cmd.addP(&pval, "present", 'p', "Present");
  cmd.addP(&hval, "help", 'h', "Print this message");
  std::string usage = R"(
usage:    OptionsHelp [options]

    Where options include:
)";
  cmd.addInfo(usage);
  
  char * targv[3];
  targv[0] = string2char("OptionsHelp");
  targv[1] = string2char(help_switch); 
  targv[2] = nullptr; 

  std::stringstream help_text;

  if(!cmd.parse(2, targv, help_text)) {
    got_text = help_text.str();    
    std::cerr << "\nParse returned false after finding \""
	      << help_switch << "\"\n";
    return false;
  }
  
  // save the returned text
  got_text = help_text.str();
  
  if(!hval) {
    std::cerr << "\nParse did not set help flag after finding \""
	      << help_switch << "\"\n";
    return false; 
  }


  return !testCompare(expected_text, help_text.str(), help_switch);
}


int main(int argc, char ** argv) {

  bool is_good = true;
  std::string got_text;
  
  if(!doTest("--help", got_text)) {
    is_good = false; 
  }
  if(!doTest("-h", got_text)) {
    is_good = false; 
  }
  if(!doTest("--h", got_text)) {
    std::cerr << "It was supposed to.\n";
    if(!testCompare(bad_option_text + expected_text, got_text, "--h")) {
      std::cerr << "But I got an incorrect error message\n";
      is_good = false; 
    }
    else {
      is_good = true;
    }
  }
  
  if(is_good) {
    std::cerr << "PASS\n";
  }
  else {
    std::cerr << "FAIL\n";    
  }
    
}
 
