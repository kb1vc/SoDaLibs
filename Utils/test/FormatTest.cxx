#include "../include/Format.hxx"
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
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

template <typename T> bool badDiff(T val, T newval, int p) {
  T diff = val - newval;
  diff = (diff > 0.0) ? diff : -diff;

  T norm = 5 * pow(10.0, T(-p)); // accommodate rounding? 

  T err = diff / val;

  bool flag = err > norm;

  if(flag) {
    std::cerr << "err = " << err << " diff = " << diff << " norm = " << norm << " val = " << val << "\n";
  }

  return flag;
  
}

template <typename T> bool checkVal(T val, const std::string & fmtstr, int precision) {
  std::stringstream ss(fmtstr);

  T newval;

  ss >> newval;

  bool ret = badDiff(val, newval, precision);
  if(ret) {
    std::cerr << val << " came out as " << newval << " string (" << fmtstr << ")\n";
  }

  return ret;
}

bool doTest(double v, int p, int w, char fmt) {
  std::string fs = SoDa::Format("%0").addF(v, fmt, w, p).str();
  //  std::cerr << fs << "]\n";
  return checkVal<double>(v, fs, p);
}

int runBigTest() {

  std::uniform_real_distribution<double> unif(0, 1.1);
  std::default_random_engine re;
  int errcount = 0;
  
  double test_v = 64.0;
  std::string formats("esg");
  int itcount = 50000; // 100000; 
  for(int i = 0; i < itcount; i += 13) {
    double test_iv = i; 
    for(double e = -18; e < 18; e++) {
      double v = test_v * pow(10.0, e);
      double iv = test_iv * pow(10.0, e);
      for(int p = 0; p < 5; p++) {
	for(int w = 0; w < 10; w++) {
	  for(auto fmt : formats) {
	    bool vt = false;
	    bool ivt = false;
	    if((vt = doTest(v, p, w, fmt)) || (ivt = doTest(iv, p, w, fmt))) {
	      errcount++;
	      std::cerr << "vt = " << ((char) (vt ? 'T' : 'F')) << " ivt = " << ((char) (ivt ? 'T' : 'F')) << " ";
	      std::cerr << "e = " << e << " v = " << v << " iv = " << iv << " p = " << p << " w = " << w << " fmt = " << fmt << "\n";
	      if(errcount > 10) {
		std::cerr << "Too many errors, time to quit.\n";
		exit(-1);
	      }
	    }
	  }
	}
      }
    }      
    test_v = unif(re);
    if(i % 1000 == 0) {
      std::cerr << i << "\n";
    }
  }
  std::cerr << "Done\n"; 

  return errcount;
}

int main(int argc, char * argv[]) {

  std::cerr << SoDa::Format("This is what 0 looks like F[%0] U[%1] I[%2] U16[%3] \n")
    .addF(0.0, 'e', 0, 0)
    .addU(0)
    .addI(0)
    .addU(0, 'X');

  auto true_str = SoDa::Format("%0").addB(true).str();
  bool bool_test = (true_str == "T");
  auto false_str = SoDa::Format("%0").addB(false).str();
  bool_test = bool_test && (false_str == "F");
  
  if(!bool_test) {
    std::cerr << "FAIL -- bool printing is broken.\n";
    exit(-1);
  }
  
  std::cerr << SoDa::Format("Testing adjacent markers [%0%1] should be [foobar]\n")
    .addS("foo")
    .addS("bar");

  auto check_str = SoDa::Format("%0 %1 %2 %3")
    .addI(33, 5, '\000', '0')
    .addI(44444, 7, ',', '0')
    .addI(55555, 7, ',')
    .addI(66666, 7)
    .str()
    ;
  auto cs_should_be = "00033 044,444  55,555   66666";
  if(cs_should_be != check_str) {
    std::cerr << "FAIL -- fill character printing is broken.\n";
    std::cerr << "got [" << check_str << "] wanted [" << cs_should_be << "]\n";
    exit(-1);
  }
  if(argc < 5) {
    int rval; 
    try {
      rval = runBigTest(); 
    }
    catch(SoDa::Exception & e) {
      std::cerr << e.what() << "\n";
      exit(-1);
    }
    if(rval > 0) {
      exit(-1);
    }
    else {
      exit(0);
    }

  }
  else {
    double v = atof(argv[1]);
    int p = atoi(argv[2]);
    int w = atoi(argv[3]);
    char fmt = argv[4][0];
    std::cerr << "Debug test v = " << v << " p = " << p << " w = " << w << " fmt = " << fmt << "\n";

    doTest(v, p, w, fmt);
  }
}
