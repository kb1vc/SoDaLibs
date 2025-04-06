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
 * @file OptionsExample.cxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 10, 2021
 */


#include <iostream>
#include <SoDa/Options.hxx>
#include <string>
#include <vector>

int main(int argc, char * argv[])
{

  //! [SO describe the command line]
  SoDa::Options cmd;  
  auto pres_arg_p = cmd.addP("presarg", 'p');
  auto bool_arg_p = cmd.add<bool>("boolarg", 'b', false, "<true/false/zero/non-zero>");
  auto str_arg_p = cmd.add<std::string>("strarg", 's', "", "<string>"); 
  auto strvec_arg_p = cmd.addV<std::string>("strvecarg", 'l', "<string>");

  auto int_arg_p = cmd.add<int>("intarg", 'i', 0, 
				"An integer argument between -5 and 5 inclusive", 
				[](int v) { return (v >= -5) && (v <= 5); },
				"Please pick something from -5 to 5.");
        
  cmd.addInfo("\nusage:\tSafeOptionsExample [options] [posargs]");
  cmd.addInfo("\n\tA simple demonstration of the safe interface to SoDa::Options parser");
  //! [SO describe the command line]

  //! [SO parse it]
  
  if(!cmd.parse(argc, argv)) exit(-1);
  //! [SO parse it]

  std::cerr << "SoDaUtils version [" << cmd.getVersion() << "]\n";
  std::cerr << "SoDaUtils git id [" << cmd.getGitID() << "]\n";

  //! [SO print the values]
  std::cout << "intarg = " << *int_arg_p << "\n";
  std::cout << "boolarg = " << *bool_arg_p << "\n";
  std::cout << "pres_arg_p = " << *pres_arg_p << "\n";
  std::cout << "str_arg = [" << *str_arg_p << "]\n";
  std::cout << "strvec_arg s = \n";
  for(auto sa : *strvec_arg_p) {
    std::cout << "\t[" << sa << "]\n";
  }
  //! [SO print the values]

  for(int i = 0; i < argc; i++) {
    std::cout << "ARG[" << i << "] = [" << argv[i] << "]\n";
  }

  //! [SO test for presence]
  
  std::cerr << (cmd.isPresent("intarg") ? "An" : "No") << " intarg option was present\n";
  //! [SO test for presence]
  
  std::cout << "posargs = \n";

  //! [SO get positional arguments]
  int i = 0;
  auto posargs = cmd.getPosArgs();
  for(auto pa : posargs) {
    std::cout << "\t" << i++ << "\t[" << pa << "]\n";
  }
  //! [SO get positional arguments]

  //! [SO reset]
  // set all values to their default state, clear the posargs list
  cmd.reset();
  //! [SO reset]
  
  //! [SO string input]
  // now try it with a string
  if(!cmd.parse(std::string("-i 3 -s \"this is a test\""))) exit(-1);

  std::cout << "intarg = " << *int_arg_p << "\n";
  std::cout << "boolarg = " << *bool_arg_p << "\n";
  std::cout << "pres_arg = " << *pres_arg_p << "\n";
  std::cout << "str_arg = [" << *str_arg_p << "]\n";
  std::cout << "strvecarg s = \n";
  //! [SO string input]
  
  exit(0);
}
