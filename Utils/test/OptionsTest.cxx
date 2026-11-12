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


// ============================================================
// Deterministic corner-case tests
// ============================================================

// Unknown long option name returns false
static bool cc_unknownLongOpt() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  std::ostringstream sink;
  return !cmd.parse(std::list<std::string>{"--notanoption", "5"}, sink);
}

// Unknown short option name returns false
static bool cc_unknownShortOpt() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  std::ostringstream sink;
  return !cmd.parse(std::list<std::string>{"-z"}, sink);
}

// Non-numeric string for int option returns false
static bool cc_badIntValue() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  std::ostringstream sink;
  return !cmd.parse(std::list<std::string>{"--count", "notanumber"}, sink);
}

// Non-numeric string for double option returns false
static bool cc_badDoubleValue() {
  SoDa::Options cmd;
  cmd.add<double>("freq", 'f', 0.0, "a frequency");
  std::ostringstream sink;
  return !cmd.parse(std::list<std::string>{"--freq", "notanumber"}, sink);
}

// User-supplied test_func rejecting a value causes parse() to return false
static bool cc_validatorRejects() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count",
    [](int v){ return v > 0; }, "must be positive");
  std::ostringstream sink;
  return !cmd.parse(std::list<std::string>{"--count", "0"}, sink);
}

// User-supplied test_func accepting a value: parse succeeds, value is set
static bool cc_validatorAccepts() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 0, "a count",
    [](int v){ return v > 0; }, "must be positive");
  bool ok = cmd.parse(std::list<std::string>{"--count", "5"});
  return ok && (*val == 5);
}

// Negative integer value is parsed correctly via the has_default=false path
static bool cc_negativeInt() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 0, "a count");
  bool ok = cmd.parse(std::list<std::string>{"--count", "-5"});
  return ok && (*val == -5);
}

// reset() restores defaults and clears isPresent; subsequent parse works
static bool cc_resetAndReparse() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 42, "a count");

  cmd.parse(std::list<std::string>{"--count", "10"});
  if (*val != 10) return false;
  if (!cmd.isPresent("count")) return false;

  cmd.reset();
  if (*val != 42) return false;
  if (cmd.isPresent("count")) return false;

  cmd.parse(std::list<std::string>{"--count", "20"});
  return (*val == 20) && cmd.isPresent("count");
}

// Positional arguments are collected independently of option values
static bool cc_positionalArgs() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 0, "a count");
  cmd.parse(std::list<std::string>{"foo", "--count", "5", "bar"});
  auto pos = cmd.getPosArgs();
  return (pos.size() == 2)
    && (pos[0] == "foo")
    && (pos[1] == "bar")
    && (*val == 5);
}

// addV accumulates each occurrence instead of overwriting
static bool cc_vectorOption() {
  SoDa::Options cmd;
  auto vec = cmd.addV<std::string>("name", 'n', "name values");
  cmd.parse(std::list<std::string>{"--name", "alice", "--name", "bob", "-n", "carol"});
  return (vec->size() == 3)
    && ((*vec)[0] == "alice")
    && ((*vec)[1] == "bob")
    && ((*vec)[2] == "carol");
}

// --help prints help text to the provided stream (parse returns false because
// "help" is not a registered option, triggering BadOptionNameException)
static bool cc_helpFlagLong() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  std::ostringstream out;
  bool result = cmd.parse(std::list<std::string>{"--help"}, out);
  return !result && (out.str().find("count") != std::string::npos);
}

// -h behaves the same as --help
static bool cc_helpFlagShort() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  std::ostringstream out;
  bool result = cmd.parse(std::list<std::string>{"-h"}, out);
  return !result && (out.str().find("count") != std::string::npos);
}

// isPresent(long_name) is true only for options that actually appeared
static bool cc_isPresentLongName() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  cmd.add<double>("freq", 'f', 0.0, "a frequency");
  cmd.parse(std::list<std::string>{"--count", "5"});
  return cmd.isPresent("count") && !cmd.isPresent("freq");
}

// isPresent(char) mirrors the long-name form
static bool cc_isPresentShortName() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  cmd.add<double>("freq", 'f', 0.0, "a frequency");
  cmd.parse(std::list<std::string>{"-c", "3"});
  return cmd.isPresent('c') && !cmd.isPresent('f');
}

// Option at end of line with no following value: parse returns true,
// value stays at default, option is NOT marked present (silent miss).
static bool cc_missingValueAtEnd() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 42, "a count");
  bool ok = cmd.parse(std::list<std::string>{"--count"});
  return ok && (*val == 42) && !cmd.isPresent("count");
}

// parse(string) tokenises correctly
static bool cc_parseFromString() {
  SoDa::Options cmd;
  auto name = cmd.add<std::string>("name", 'n', "", "a name");
  bool ok = cmd.parse(std::string("--name hello"));
  return ok && (*name == "hello");
}

// parse(string) with a double-quoted value preserves internal spaces
static bool cc_parseFromStringWithQuotes() {
  SoDa::Options cmd;
  auto name = cmd.add<std::string>("name", 'n', "", "a name");
  bool ok = cmd.parse(std::string("--name \"hello world\""));
  return ok && (*name == "hello world");
}

// Empty argument list: all values remain at defaults, nothing is present
static bool cc_emptyArgList() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 99, "a count");
  bool ok = cmd.parse(std::list<std::string>{});
  return ok && (*val == 99) && !cmd.isPresent("count");
}

// Short-form option sets value correctly
static bool cc_shortFormOption() {
  SoDa::Options cmd;
  auto val = cmd.add<int>("count", 'c', 0, "a count");
  bool ok = cmd.parse(std::list<std::string>{"-c", "7"});
  return ok && (*val == 7);
}

// addP presence flag: false when absent, true when flag appears on command line
static bool cc_presenceFlagOnOff() {
  SoDa::Options cmd;
  auto flag = cmd.addP("verbose", 'v', "verbose mode");

  cmd.parse(std::list<std::string>{});
  if (*flag) return false;

  cmd.reset();

  cmd.parse(std::list<std::string>{"--verbose"});
  return *flag;
}

// parseKeyValue parses comma-separated key=value pairs
static bool cc_keyValueParse() {
  SoDa::Options cmd;
  auto count = cmd.add<int>("count", 'c', 0, "a count");
  auto name  = cmd.add<std::string>("name", 'n', "", "a name");
  bool ok = cmd.parseKeyValue(std::string("count=5, name=hello"));
  return ok && (*count == 5) && (*name == "hello");
}

// parseKeyValue with an unknown key returns false
static bool cc_keyValueUnknownKey() {
  SoDa::Options cmd;
  cmd.add<int>("count", 'c', 0, "a count");
  std::ostringstream sink;
  return !cmd.parseKeyValue(std::string("count=5, unknown=hello"), sink);
}

// Mix of short and long forms in one parse call
static bool cc_mixedShortAndLong() {
  SoDa::Options cmd;
  auto count = cmd.add<int>("count", 'c', 0, "a count");
  auto freq  = cmd.add<double>("freq", 'f', 0.0, "a frequency");
  auto name  = cmd.add<std::string>("name", 'n', "", "a name");
  bool ok = cmd.parse(std::list<std::string>{"--count", "3", "-f", "14.225", "--name", "kb1vc"});
  return ok && (*count == 3) && doubleEQ(*freq, 14.225) && (*name == "kb1vc");
}

// numPosArgs() returns the right count
static bool cc_numPosArgs() {
  SoDa::Options cmd;
  cmd.parse(std::list<std::string>{"a", "b", "c"});
  return cmd.numPosArgs() == 3;
}

bool runCornerCaseTests() {
  struct Entry { const char * name; bool (*fn)(); };
  static const Entry tests[] = {
    { "unknown long option",           cc_unknownLongOpt       },
    { "unknown short option",          cc_unknownShortOpt      },
    { "bad int value",                 cc_badIntValue          },
    { "bad double value",              cc_badDoubleValue       },
    { "validator rejects value",       cc_validatorRejects     },
    { "validator accepts value",       cc_validatorAccepts     },
    { "negative int (signed path)",    cc_negativeInt          },
    { "reset and re-parse",            cc_resetAndReparse      },
    { "positional args",               cc_positionalArgs       },
    { "vector option (addV)",          cc_vectorOption         },
    { "--help flag",                   cc_helpFlagLong         },
    { "-h flag",                       cc_helpFlagShort        },
    { "isPresent (long name)",         cc_isPresentLongName    },
    { "isPresent (short name)",        cc_isPresentShortName   },
    { "missing value at end of line",  cc_missingValueAtEnd    },
    { "parse from string",             cc_parseFromString      },
    { "parse from string with quotes", cc_parseFromStringWithQuotes },
    { "empty arg list",                cc_emptyArgList         },
    { "short form option",             cc_shortFormOption      },
    { "presence flag on/off",          cc_presenceFlagOnOff    },
    { "parseKeyValue",                 cc_keyValueParse        },
    { "parseKeyValue unknown key",     cc_keyValueUnknownKey   },
    { "mixed short and long forms",    cc_mixedShortAndLong    },
    { "numPosArgs",                    cc_numPosArgs           },
  };

  bool all_pass = true;
  std::cerr << "\n--- Corner case tests ---\n";
  for (const auto & t : tests) {
    bool passed = false;
    try {
      passed = t.fn();
    } catch (const std::exception & e) {
      std::cerr << "  EXCEPTION in [" << t.name << "]: " << e.what() << "\n";
    } catch (...) {
      std::cerr << "  UNKNOWN EXCEPTION in [" << t.name << "]\n";
    }
    std::cerr << (passed ? "  pass: " : "  FAIL: ") << t.name << "\n";
    if (!passed) all_pass = false;
  }
  std::cerr << "--- Corner cases " << (all_pass ? "PASSED" : "FAILED") << " ---\n\n";
  return all_pass;
}

int main(int argc, char ** argv) {

  SoDa::Options cmd;
  auto num_trials_p = cmd.add<int>("trials", 't', 1000, "Number of test cases to try");

  if(!cmd.parse(argc, argv)) exit(-1);

  std::cerr << "Number of trials [" << *num_trials_p << "]\n";

  int max_opts = 50;

  bool corner_pass = runCornerCaseTests();

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
  if(is_good && corner_pass) std::cout << "PASS\n";
  else std::cout << "FAIL\n";
}
 
