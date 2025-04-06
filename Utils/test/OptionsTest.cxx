#include "../include/Format.hxx"
#include "../include/Options.hxx"
#include <iostream>
#include <cmath>
#include <type_traits>

int si_val;
int u_val;
float f0_val;
float f1_val; 
bool b_val;
std::string s_val;
std::vector<std::string> s_val_list;
std::vector<int> a_val_list;
bool pres_val;
std::string sind_val;

bool testPosArgs(SoDa::Options cmd) {
  si_val = 0; 
  std::list<std::string> arglist = { 
    "--sint", "-3",
    "--uint", "3", 
    "--fva0", "-1.1",
    "--fva1", "1.1",
    "--sva", "\"--this is a test\"",
    "--s-ind", "'--args=\"type=b200,serial=c00ld00d\"'"
  };
  

  std::list<std::string> poslist = {
    "po0", "po1", "po2" };
  
  auto tmpposlist = poslist;

  arglist.merge(poslist);

  bool is_good = true; 
  if(!cmd.parse(arglist)) {
    std::cerr << "testPosArgs bad parse\n";
    is_good = false; 
  }

  // check the aval.
  if(si_val != -3) {
    std::cerr << "testPosArgs bad si_val\n";
    is_good = false;
  }
  if(u_val != 3) {
    std::cerr << "testPosArgs bad u_val\n";
    is_good = false;
  }
  if(std::fabs(f0_val + 1.1) > 1e-6) {
    std::cerr << "testPosArgs bad f0_val\n";
    is_good = false;
  }
  if(std::fabs(f1_val - 1.1) > 1e-6) {
    std::cerr << "testPosArgs bad f1_val\n";
    is_good = false;
  }
  if(sind_val != std::string("'--args=\"type=b200,serial=c00ld00d\"'")) {
    std::cerr << "testPosArgs bad value for s-ind : [" << sind_val << "]\n";
    is_good = false; 
  }
  
  if(s_val != "\"--this is a test\"") {
    std::cerr << "testPosArgs bad value for sva : [" << s_val << "]\n";
    is_good = false; 
  }
  if(cmd.numPosArgs() != 3) {
    std::cerr << "testPosArgs got bad arg count\n";
    is_good = false; 
  }
  
  auto pags = cmd.getPosArgs();
  if(pags.size() != 3) {
    std::cerr << "testPosArgs got bad arglist count\n";
    is_good = false; 
  }

  int i = 0;
  for(auto refa : poslist) {
    auto pa = cmd.getPosArg(i); 

    if(pa.compare(refa) != 0) {
      std::cerr << SoDa::Format("testPosArgs getPosArg got [%0] should be [%1]\n")
	.addS(pa)
	.addS(refa);
    is_good = false; 
    }
    if(pags[i].compare(refa) != 0) {
      std::cerr << SoDa::Format("testPosArgs getPosArgs[%2] got [%0] should be [%1]\n")
	.addS(pa)
	.addS(refa)
	.addI(i);
      is_good = false; 
    }
    i++; 
  }
  
  return is_good; 
}

int main(int argc, char ** argv) {
  // create a mailbox
  SoDa::Options cmd;

  cmd.addP(&pres_val, "pres", 'p')
    .add<bool>(&b_val, "boo", 'b')
    .add<int>(&si_val, "sint", 'I')
    .add<int>(&u_val, "uint", 'U')    
    .add<float>(&f0_val, "fva0", 'f')
    .add<float>(&f1_val, "fva1", 'F')    
    .add(&s_val, "sva", 's')
    .add(&sind_val, "s-ind", 'c')    
    .addV<std::string>(&s_val_list, "sla", 'l')
    .addV<int>(&a_val_list, "ala", 'L');


  bool is_good = true;
  std::cout << "argc = " << argc << "\n";
  if(argc < 2) {
    std::cout << "We are in here.\n";
    is_good = is_good && testPosArgs(cmd); 
  }  
  else {
    std::cout << "Why are we here?\n";
    cmd.parse(argc, argv);
    std::cout << "!!!! s_val = ??? [" << s_val << "]\n";
  }
  
  if(is_good) std::cout << "PASS\n";
  else std::cout << "FAIL\n";
}
 
