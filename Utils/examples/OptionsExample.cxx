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
  int int_arg;
  bool bool_arg, pres_arg;
  std::string str_arg;
  std::vector<std::string> strvec_arg; 

  //! [describe the command line]
  SoDa::Options cmd;  
  cmd
    .addP(&pres_arg, "presarg", 'p')    
    .add<bool>(&bool_arg, "boolarg", 'b', false, "<true/false/zero/non-zero>")
    .add<std::string>(&str_arg, "strarg", 's', "", "<string>") // , "Not Specified")
    .addV<std::string>(&strvec_arg, "strvecarg", 'l', "<string>")

    .add<int>(&int_arg, "intarg", 'i', 0,
	       "An integer argument between -5 and 5 inclusive", 
	       [](int v) { return (v >= -5) && (v <= 5); },
	       "Please pick something from -5 to 5.")
        
    .addInfo("\nusage:\tOptionsExample [options] [posargs]")
    .addInfo("\n\tA simple demonstration of the SoDa::Options parser");
  //! [describe the command line]

  //! [parse it]
  
  if(!cmd.parse(argc, argv)) exit(-1);
  //! [parse it]

  std::cerr << "SoDaUtils version [" << cmd.getVersion() << "]\n";
  std::cerr << "SoDaUtils git id [" << cmd.getGitID() << "]\n";
  
  std::cout << "intarg = " << int_arg << "\n";
  std::cout << "boolarg = " << bool_arg << "\n";
  std::cout << "pres_arg = " << pres_arg << "\n";
  std::cout << "str_arg = [" << str_arg << "]\n";
  std::cout << "strvecarg s = \n";
  for(auto sa : strvec_arg) {
    std::cout << "\t[" << sa << "]\n";
  }

  for(int i = 0; i < argc; i++) {
    std::cout << "ARG[" << i << "] = [" << argv[i] << "]\n";
  }
  std::cerr << (cmd.isPresent("intarg") ? "An" : "No") << " intarg option was present\n";
  
  std::cout << "posargs = \n";
  for(int i = 0; i < cmd.numPosArgs(); i++) {
    std::cout << "\t" << i << "\t" << cmd.getPosArg(i) << "\n";
  }

  // now try it with a string
  if(!cmd.parse(std::string("-i 3 -s \"this is a test\""))) exit(-1);

  std::cout << "intarg = " << int_arg << "\n";
  std::cout << "boolarg = " << bool_arg << "\n";
  std::cout << "pres_arg = " << pres_arg << "\n";
  std::cout << "str_arg = [" << str_arg << "]\n";
  std::cout << "strvecarg s = \n";
  
  exit(0);
}
