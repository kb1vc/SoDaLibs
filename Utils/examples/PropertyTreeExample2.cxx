#include <SoDa/PropertyTree.hxx>
#include <SoDa/Format.hxx>
#include <SoDa/Options.hxx>

#include <iostream>
#include <fstream>
#include <thread>
#include <functional>
#include <chrono>
#include <random>
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

// This is *exactly* like PropertyTreeExample2, except it tests
// the ability to independently link the jsoncpp library, if necessary.
// If the SoDaUtils lib build is done incorrectly, compiling this program
// will result in a duplicate definition of various json methods.
//
#include <json/json.h>


void testJsonCPP()
{
  Json::Value root;
  root["first"] = 1;
  root["second"] = 2;
  std::cout << root << "\n";
}

int main(int argc, char * argv[]) {
  SoDa::Options cmd; 
  
  if(!cmd.parse(argc, argv) || (cmd.numPosArgs() < 1)) {
    std::cerr << "Missing test file name.\n";
    exit(-1); 
  }
  
  std::string tstname = cmd.getPosArg(0);
  
  SoDa::PropertyTree ptree;

  std::string string_prop("This:String");
  std::string double_prop("That:l1:double");
  std::string long_prop("That:long");
  std::string ulong_prop("That:l2:unsigned long");
    
  
  double t_double = 3.5;
  unsigned long t_ulong = 35;
  long t_long = -75;
  std::string t_string = "Test string";
  ptree.put(t_double, double_prop, true, true);
  ptree.put(t_long, long_prop, true, true);
  ptree.put(t_ulong, ulong_prop, true, true);  
  ptree.put(t_string, string_prop, true, true);

  ptree.writeFile(tstname);

  SoDa::PropertyTree rtree(tstname);
  
  std::string r_string;
  double r_double;
  unsigned long r_ulong;
  long r_long; 
  rtree.get(r_double, double_prop, true);
  rtree.get(r_long, long_prop, true);
  rtree.get(r_ulong, ulong_prop, true);  
  rtree.get(r_string, string_prop, true);

  bool failed = false; 
  if(r_string != t_string) {
    std::cerr << SoDa::Format("Mismatch retrieving %0 got [%1] should have been [%2]\n")
      .addS(string_prop)
      .addS(r_string)
      .addS(t_string)
      ;
    failed = true; 
  }

  if(r_long != t_long) {
    std::cerr << SoDa::Format("Mismatch retrieving %0 got [%1] should have been [%2]\n")
      .addS(long_prop)
      .addI(r_long)
      .addI(t_long)
      ;
    failed = true; 
  }

  if(r_ulong != t_ulong) {
    std::cerr << SoDa::Format("Mismatch retrieving %0 got [%1] should have been [%2]\n")
      .addS(ulong_prop)
      .addU(r_ulong)
      .addU(t_ulong)
      ;
    failed = true; 
  }

  if(r_double != t_double) {
    std::cerr << SoDa::Format("Mismatch retrieving %0 got [%1] should have been [%2]\n")
      .addS(double_prop)
      .addF(r_double)
      .addF(t_double)
      ;
    failed = true; 
  }


  // now let's see if it faults on bad paths.

  try {
    double v;
    bool r = rtree.get(v, "This:Missingprop", true);
  }
  catch ( SoDa::PropertyTree::PropertyNotFound e) {
    std::cerr << SoDa::Format("Got what I expected -- [%0]\n").addS(e.what());
  }
  catch ( ... ) {
    failed = true; 
  }

  try {
    double v;
    bool r = rtree.get(v, string_prop, true);
  }
  catch ( SoDa::PropertyTree::BadPropertyType e) {
    std::cerr << SoDa::Format("Got what I expected -- [%0]\n").addS(e.what());
  }
  catch ( SoDa::Exception e) {
    std::cerr << SoDa::Format("Didn't expect this exception -- [%0]\n").addS(e.what());
    failed = true; 
  }

  // try to read a bad file.
  std::ofstream badfile("badfile");
  if(badfile.is_open()) {
    badfile << "This is not a json file 234251 #@()%@\n";
    badfile.close();
    std::ofstream badinfile("badfile");
    try {
      SoDa::PropertyTree bad("badfile");
    }
    catch ( SoDa::PropertyTree::FileParseError e) {
      std::cerr << SoDa::Format("Got what I expected -- [%0]\n").addS(e.what());      
    }
    catch ( SoDa::Exception e) {
      std::cerr << SoDa::Format("Didn't expect this exception -- [%0]\n").addS(e.what());
      failed = true; 
    }
  }
  else {
    std::cerr << "Couldn't open test file.  This is a problem with the test program.\n";
    failed = true; 
  }

  // try to read a file that doesn't exist
  try {
    SoDa::PropertyTree rtr("/usr/thisisunlikelytoeverexist");
  }
  catch ( SoDa::PropertyTree::FileNotFound e) {
    std::cerr << SoDa::Format("read missing file: Got what I expected -- [%0]\n").addS(e.what());
  }
  catch ( SoDa::Exception e) {
    std::cerr << SoDa::Format("read missing file: Didn't expect this exception -- [%0]\n").addS(e.what());
    failed = true; 
  }

  // write a file in the wrong place
  try {
    rtree.writeFile("/usr/cannot_write_here_as_anybody_but_root");
  }
  catch ( SoDa::PropertyTree::FileNotWriteable e) {
    std::cerr << SoDa::Format("write non-writeable file: Got what I expected -- [%0]\n").addS(e.what());
  }
  catch ( SoDa::Exception e) {
    std::cerr << SoDa::Format("write non-writeable file: Didn't expect this exception -- [%0]\n").addS(e.what());
    failed = true; 
  }

  std::string badprop = double_prop + ":nothere";

  // let's get from a property that doesn't exist
  try {
    // this should not throw
    double td;
    if(rtree.get(td, badprop, false)) {
      std::cerr << "Attempted to get from a missing property, and got wrong return value.\n";
      failed = true; 
    }
  }
  catch ( SoDa::Exception e) {
    std::cerr << SoDa::Format("get badprop: Didn't expect this exception -- [%0]\n").addS(e.what());
    failed = true; 
  }
  

  // now let's put to a property that doesn't exist
  try {
    // this should throw
    rtree.put(3.5, badprop, false, true);
    failed = true;     
  }
  catch ( SoDa::PropertyTree::PropertyNotFound e) {
    std::cerr << SoDa::Format("put badprop: Got what I expected -- [%0]\n").addS(e.what());
  }
  catch ( SoDa::Exception e) {
    std::cerr << SoDa::Format("put badprop: Didn't expect this exception -- [%0]\n").addS(e.what());
    failed = true; 
  }

  // now let's put to a property that doesn't exist but it is ok.
  try {
    // this should throw
    if(rtree.put(3.5, badprop, false, false)) {
      std::cerr << "Tried to put a bad property but it worked.\n";
      failed = true;       
    }
  }
  catch ( SoDa::Exception e) {
    std::cerr << SoDa::Format("put badprop/nothrow: Didn't expect this exception -- [%0]\n").addS(e.what());
    failed = true; 
  }


  testJsonCPP();
  
  
  if(failed) {
    std::cerr << "FAILED\n";
  }
  else {
    std::cerr << "PASSED\n";
  }
  
  
}
