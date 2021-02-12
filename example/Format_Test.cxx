#include <SoDa/Format.hxx>
#include <iostream>

int main() {
  int i = 5;
  double ef = 32157.5;
  char c = 'T';
  std::string fred("Fred");

  //! [printing some stuff]    
  std::cout << SoDa::Format("print 5 like this:  %0\n"
			    "let's print 5 again %0\n"
			    "%1 looks much better in engineering notation %2\n"
			    "Sometimes I just want to print a %% sign like this %0%%\n"
			    "I think that shirt he's wearing fits %3 to a %4\n"
			    "We can print hex too. Decimal %5 is %6 in hex\n")
    .addI(i)
    .addF(ef, 'f')    
    .addF(ef, 'e')
    .addS(fred)
    .addC(c)
    .addU(55, 'd', 4)
    .addU(55, 'x', 4);  
  
  //! [printing some stuff]

  //! [print one by one]
  std::cout << SoDa::Format("i = %0\n").addI(i);
  std::cout << SoDa::Format("ef = \n\t(f format) %0 or \n\t(g format) %1 or \n\t(s format) %2  or \n\t(e format) %3\n")
    .addF(ef, 'f').addF(ef, 'g').addF(ef, 's').addF(ef, 'e');
  //! [print one by one]
  
  std::cout << SoDa::Format("int [%0] double f [%1] double g [%2] "
			    "double e [%3] string [%4] "
			    "char [%5] char6[%6] char7[%7] "
			    "char8[%8] char9[%9] char10[%10] char11[%11]\n")
    .addI(i).addF(ef, 'f').addF(ef, 'g').addF(ef, 'e').addS(fred).addC(c)
    .addC('6')
    .addC('7')
    .addC('8')
    .addC('9')
    .addC('A')
    .addC('B');    

  //! [create a format object]
  SoDa::Format sft("Avogadro's number: %0\n"); 
  //! [create a format object]


  //! [print avogadro's number]
  // Print Avogadro's number the way we all remembered from high school. 
  double av = 6.02214076e23;
  std::cout << sft.addF(av, 's');

  // But all that 10^23 jazz is the result of the evil CGS system.
  // Let's do this the way a respectable MKS user would have wanted it.
  // first, let's reuse our format:
  //! [reset the format]
  sft.reset();
  //! [reset the format]  
  // and now print
  std::cout << "Here's how right thinking people write "
            << sft.addF(av, 'e');
  //! [print avogadro's number]  

  // we'll reuse this format in the test loop
  SoDa::Format format("%0 p = %1 : [%2]\n");

  // let's test and demonstrate each of the four FP formats
  std::list<char> fs = {'f', 'g', 's', 'e'};
  
  for(i = 0; i < 8; i++) {
    std::cout << "---------\n" << ef << "\n--------------\n";    
    for(char fmt : fs) {
      //! [reusing a format and multiple adds]
      for(int p = 1; p < 4; p++) {
        std::cout << format.reset().addC(fmt).addI(p).addF(ef, fmt, p+3, p);
      }
      //! [reusing a format and multiple adds]      
    }
    ef = ef * 0.1;
    std:: cout << "\n\n";
  }

  // now exercise the rounding
  double base_val = 111111.0;
  double incr = 0.001;
  for(double v = 0.0; v <= 4.0; v += incr) {
    std::cout << SoDa::Format("RT %0 %1\n").addF(v, 'f', 8, 6).addF(base_val + v, 'e', 3);
  }
    
  SoDa::Format oops("%0\n");

  std::cout << oops.addI(3).addI(4);
  
}
