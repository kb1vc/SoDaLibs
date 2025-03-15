#include "../include/Format.hxx"
#include <string>
#include <iostream>
#include <math.h>
#include <cmath>
#include <stdio.h>

int main(int argc, char * argv[]) {
  // make sure that printing NaN or Inf does the right
  // thing -- (doesn't hang!)

  std::string foo = SoDa::Format("ok %0 inf %1 nan %2")
    .addF(1.0, 'f', 6)
    .addF(1.0 / 0.0, 'f', 4)
    .addF(std::sqrt(-1.0), 'f', 4)
    .str();

  if(foo != "ok 1.000000 inf inf  nan nan ") {
    std::cout << "[" << foo << "]\n";    
    exit(-1); 
  }

  exit(0);
}
