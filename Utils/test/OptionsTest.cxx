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
#include <random>

class TestCase {
public:
  TestCase() {
    makeOptSel();
    switches.insert('h');
  }

  virtual void set(SoDa::Options & cmd) = 0; 
  
  virtual void emit(std::list<std::string> & arglist) = 0;

  virtual bool check() = 0;

  virtual void dump() = 0;
  
  char genChar() {
    std::uniform_int_distribution<int> tdist(0, 100000);    
    auto idx = tdist(gen);
    char sw  = 'a' + char(idx % 26);
    while(switches.find(sw) != switches.end()) {
      sw = 'a' + char(idx % 26);
      idx = tdist(gen);
    }
    switches.insert(sw);

    return sw; 
  }

  std::string genString(size_t minlen, size_t maxlen) {
    std::uniform_int_distribution<int> tdist(minlen, maxlen);
    auto slen = tdist(gen);
    
    std::uniform_int_distribution<int> sdist(0,legal_chars.size());
    std::string ret;
    char c1; 
    do {
      auto idx = sdist(gen); 
      c1 = legal_chars[idx];
    } while((c1 == '_') || (c1 == '-')); 
    ret.push_back(c1); 
    for(int i = 1; i < slen; i++) {
      
      auto idx = sdist(gen);
      char c = legal_chars[idx]; 
      ret.push_back(c); 
    }

    return ret; 
  }

  void makeOptSel() {
    long_s = genString(5, 12);
    short_s = genChar();
  }
  
  void addCommand(std::list<std::string> & arglist) {
    std::uniform_real_distribution<double> dist(0,1);
    bool s_or_l = (dist(gen) > 0.5);
    if(s_or_l) {
      arglist.push_back(SoDa::Format("--%0").addS(long_s).str());
    }
    else {
      arglist.push_back(SoDa::Format("-%0").addC(short_s).str());      
    }
  }

  static void clearSwitches() {
    switches.clear();
    switches.insert('h');
  }
  
  static std::string legal_chars; 
  static std::default_random_engine gen;

  static uint32_t inst_counter; 
  
  static std::set<char> switches;

  std::string long_s;
  char short_s;

}; 

typedef std::unique_ptr<TestCase> TestCaseUPtr;

std::string TestCase::legal_chars("abcdefghijklmnopqrstuvwxyz0123456789_-ABCDEFGHIJKLMNOPQRSTUVWXYZ");
uint32_t TestCase::inst_counter = 0;
std::default_random_engine TestCase::gen{std::random_device{}()};
std::set<char> TestCase::switches;

class BoolTest : public TestCase {
public:
  BoolTest() : TestCase() {
    dist = std::uniform_real_distribution<double>(0, 1);
  }
  
  void set(SoDa::Options & cmd) {
    // pick a random value
    req_val = (dist(gen) > 0.5);

    def_val = (dist(gen) > 0.5);

    val = cmd.add<bool>(long_s, short_s, def_val, "gimme a bool!"); 
  }
  
  void emit(std::list<std::string> & arglist) override {
    if(dist(gen) > 0.66) { // we may not supply an argument at all...
      // so requred val is def_val
      req_val = def_val; 
      return; 
    }    

    // short or long?
    addCommand(arglist);
    // add a T or F or 0 or 1
    auto vs = dist(gen);
    if(req_val) {
      if(vs < 0.5) {
	arglist.push_back("T");
      }
      else {
	arglist.push_back("1");
      }
    }
    else {
      if(vs < 0.5) {
	arglist.push_back("F");
      }
      else {
	arglist.push_back("0");
      }
    }
  }
  
  bool check() {
    return req_val == *val; 
  }


  void dump() {
    std::string rtwr = (req_val == *val) ? "correct!" : "wrong!";
    
    std::cerr << SoDa::Format("Bool [%0] short [%1] expected [%2] default [%3] got [%4]  %5\n")
      .addS(long_s)
      .addC(short_s)
      .addB(req_val)
      .addB(def_val)
      .addB(*val)
      .addS(rtwr)
      ;
  }
  
  std::shared_ptr<bool> val;

  bool supply_arg;
  
  bool req_val;

  bool def_val; 

  std::uniform_real_distribution<double> dist; 
};

class PresentTest : public TestCase {
public:
  PresentTest() : TestCase() {
    dist = std::uniform_real_distribution<double>(0, 1);
  }
  
  void set(SoDa::Options & cmd) {
    // std::cerr << SoDa::Format("PresentTest::set adding --%0 -%1\n")
    //   .addS(long_s).addC(short_s);
    val = cmd.addP(long_s, short_s, "present or not");
  }
  
  void emit(std::list<std::string> & arglist) override {
    if(dist(gen) > 0.66) { // we may not supply an argument at all...
      // so requred val is def_val
      // std::cerr << SoDa::Format("PresentTest::emit will not emit %0\n")
      // 	.addS(long_s);
      req_val = false;
      return; 
    }
    else {
      // std::cerr << SoDa::Format("PresentTest::emit will emit %0\n")
      // 	.addS(long_s);
      addCommand(arglist);
      req_val = true; 
    }
  }
  
  bool check() {
    bool ret = req_val == *val; 
    if(!ret) {
      dump();
    }
    return ret;
  }


  void dump() {
    std::string rtwr = (req_val == *val) ? "correct!" : "wrong!";
    
    std::cerr << SoDa::Format("Present [%0] short [%1] expected [%2]  got [%4]  %5\n")
      .addS(long_s)
      .addC(short_s)
      .addB(req_val)
      .addB(*val)
      .addS(rtwr)
      ;
  }
  
  std::shared_ptr<bool> val;

  bool supply_arg;
  
  bool req_val;

  bool def_val; 

  std::uniform_real_distribution<double> dist; 
};

bool intEQ(int a, int b) {
  return a == b;
}

bool doubleEQ(double a, double b) {
  auto diff = fabs(b - a);

  bool ret = fabs(diff) < fabs(1e-4 * (b + a));
  // on rare occasions (a = 0, b = 0) we should return true
  if(a == b) ret = true; 
  
  if(!ret) {
    std::cerr << SoDa::Format("Got unequal doubles [%0] [%1] diff [%2]\n")
      .addF(a, 'e', 14)
      .addF(b, 'e', 14)
      .addF(diff, 'e', 14)
      ;
  }
  
  return ret; 
}


enum ScalarType { DOUBLE, INT };
template<typename T, typename DT, ScalarType st, bool (*iseq)(T, T)>
class ScalarTest : public TestCase {
public:
  ScalarTest() : TestCase() {
    dist = DT(-1000, 1000);
  }
  
  void set(SoDa::Options & cmd) {
    // pick a random value
    req_val = dist(gen);

    def_val = dist(gen);
    std::string ststr = (st == DOUBLE) ? "double" : "int";    

    val = cmd.add<T>(long_s, short_s, def_val, "gimme a " + ststr); 
  }
  
  void emit(std::list<std::string> & arglist) override {
    if(dist(gen) > 0.66) { // we may not supply an argument at all...
      // so requred val is def_val
      req_val = def_val;
      return; 
    }     

    // short or long?
    addCommand(arglist);
    // add a T or F or 0 or 1
    auto vs = dist(gen);

    std::ostringstream ost;
    ost << req_val;

    arglist.push_back(ost.str());
  }
  
  bool check() {
    return iseq(req_val, *val); 
  }


  void dump() {
    std::ostringstream rvs, dvs, vvs;
    rvs << req_val;
    dvs << def_val;
    vvs << *val;

    std::string rtwr = iseq(req_val, *val) ? "correct!" : "wrong!";
    std::cerr << SoDa::Format("Scalar [%0] short [%1] expected [%2] default [%3] got [%4]  %5\n")
      .addS(long_s)
      .addC(short_s)
      .addS(rvs.str())
      .addS(dvs.str())
      .addS(vvs.str())
      .addS(rtwr)
      ;
  }
  
  std::shared_ptr<T> val;
  T req_val;
  T def_val;

  DT dist; 
};

template<typename T, typename TD, ScalarType st, bool (*iseq)(T,T)>
std::unique_ptr<ScalarTest<T,TD, st, iseq>> makeScalar() {
  auto rp =  new ScalarTest<T, TD, st, iseq>;
  return std::unique_ptr<ScalarTest<T, TD, st, iseq>>(rp); 
}

TestCaseUPtr makeCase() {
  std::uniform_int_distribution<> fdist(0, 3);
  auto sel = fdist(TestCase::gen);
  switch (sel) {
  case 0: 
    return std::unique_ptr<BoolTest>(new BoolTest);
    break;
  case 1:
    return makeScalar<int, std::uniform_int_distribution<int>, INT, intEQ>(); 
    break;
  case 2:
    return makeScalar<double, std::uniform_real_distribution<double>, DOUBLE, doubleEQ>();     
    break; 
  case 3:
    return std::unique_ptr<PresentTest>(new PresentTest);
    break; 
  default:
    return makeScalar<double, std::uniform_real_distribution<double>, DOUBLE, doubleEQ>();     
  }
}

std::list<TestCaseUPtr> makeCases(int max_opts) {
  std::uniform_int_distribution<int> na_dist(1,max_opts);
  std::list<TestCaseUPtr> ret; 
  auto num_args = na_dist(TestCase::gen);

  for(int i = 0; i < num_args; i++) {
    ret.push_back(makeCase());
  }
  
  return ret; 
}


std::list<std::string> makeArgV(std::list<TestCaseUPtr> & tc) {
  std::list<std::string> arg_list;
  arg_list.push_back("test_case");  

  for(auto & c : tc) {
    c->emit(arg_list);
  }

  return arg_list; 
}

void dumpArgV(const std::list<std::string> & arg_list) {
  std::cerr << "Command [";
  for(auto & a : arg_list) {
    std::cerr << SoDa::Format("%0 ").addS(a);
  }
  std::cerr << "]\n";
}


int main(int argc, char ** argv) {

  SoDa::Options cmd;
  auto num_trials_p = cmd.add<int>("trials", 't', 1000, "Number of test cases to try");

  if(!cmd.parse(argc, argv)) exit(-1);

  std::cerr << "Number of trials [" << *num_trials_p << "]\n";
  
  int max_opts = 50;

  
  bool is_good = true; 
  for(int i = 0; i < *num_trials_p; i++) {
    if(!is_good) {
      std::cerr << "Failed after " << i << " trials\n";
      break;
    }
    SoDa::Options cmd; 
    if((i % 10000) == 0) {
      std::cerr << SoDa::Format("Trial %0/%1\r")
	.addI(i, 10)
	.addI(*num_trials_p)
	;
    }
    TestCase::clearSwitches();
    auto cases = makeCases(10);
    for(auto & c : cases) {
      c->set(cmd);
    }

    auto arg_list = makeArgV(cases);

    if(!cmd.parse(arg_list))  {
      std::cerr << "Error in parsing command:\n";
      is_good = false;
      dumpArgV(arg_list);
    }
    else {
      for(auto & c : cases) {
	if(!c->check()) {
	  is_good = false;
	  std::cerr << "Error in expected value for command:\n";
	  cmd.printHelp(std::cerr);
	  dumpArgV(arg_list);
	  c->dump();
	}
      }
    }
  }

  std::cout << "\n";
  if(is_good) std::cout << "PASS\n";
  else std::cout << "FAIL\n";
}
 
