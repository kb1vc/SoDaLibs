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

#include <SoDa/Format.hxx>
#include <iostream>

/**
 * @brief Extend the Format class to print a string, backwards.
 * 
 * So .addSBW("foo") would produce "oof"
 * 
 * This extends the Format class and preserves 
 * the "format methods return a format" scheme. 
 */
class MyFormat : public SoDa::Format_ext<MyFormat> {
public:
  MyFormat(const std::string & fmt_string) : Format_ext<MyFormat>(fmt_string, this) {
  }

  MyFormat & addSBW(const std::string & v) {
    std::string rv;
    for(auto c : v) {
      rv = c + rv;
    }
    addS(rv); 
    return *this;
  }
};

template<typename FMT>
void testFormatParam(FMT & fmt, int i) {
  std::cout << fmt.addI(i * i).addSBW("foo"); 
}

int main() {
  SoDa::Format sft("Avogadro's number: %0\n"); 

  // Print Avogadro's number the way we all remembered from high school. 
  double av = 6.02214076e23;
  std::cout << sft.addF(av, 's');

  // But all that 10^23 jazz is the result of the evil CGS system.
  // Let's do this the way a respectable MKS user would have wanted it.
  std::cout << "Here's how right thinking people write "
            << sft.reset().addF(av, 'e');

  std::string test_string("TestString");
  std::cout << MyFormat("String forward %0 string backward %1 string forward again %2\n")
    .addS(test_string)
    .addSBW(test_string)
    .addS(test_string);

  MyFormat mfmt("this is an int %0  this is a backward string %1\n");

  testFormatParam(mfmt, 3);
  
  // print a big number.
  int bignum = 123456789;
  std::cout << SoDa::Format("Two ways to see a big number [%0] and [%1]\n")
    .addI(bignum, 8)
    .addI(bignum, 14, ',');
    
  // now print some hex stuff
  unsigned long lhex = 0xfedcba9876543210;
  std::cout << SoDa::Format("Hard to read [%0], easy to read [%1]\n")
    .addU(lhex, 'x', 30)
    .addU(lhex, 'X', 28, '_', 4);
  
  unsigned int v = 0x0; 
  std::cout << SoDa::Format("0 is decimal [%0] octal [%1] hex [%2]\n")
    .addU(v)
    .addU(v, 'o')
    .addU(v, 'h');
  
  v = 07543;
  std::cout << SoDa::Format("07543 is decimal [%0] octal [%1] hex [%2]\n")  
    .addU(v)
    .addU(v, 'o')
    .addU(v, 'h');
}
