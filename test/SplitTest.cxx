#include "../include/Utils.hxx"
#include <iostream>

std::string comma_test = "this,that,the other thing";
std::vector<std::string> comma_ref = {"this", "that", "the other thing" };


int main() {
  
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
  
  if(pass) {
    std::cerr << "PASS\n";
  }
  else {
    std::cerr << "FAIL\n";
  }
}
