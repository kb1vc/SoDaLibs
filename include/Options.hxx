#pragma once
#include "UtilsBase.hxx"
#include <memory>
/*
BSD 2-Clause License

Copyright (c) 2021, 2022, Matt Reilly - kb1vc
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
 * @file Options.hxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 10, 2021
 */

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include <exception>
#include <stdexcept>
#include <functional>
#include <algorithm>

namespace SoDa {

/**
 * @page Options  A simple command line parser
 * 
 * Options is a class that allows the programmer to specify
 * command line options (like --help, --out, --enable-deep-fry --set-sauce=Memphis)
 * and parse the (argc, argv) input line.  There are other ways to do
 * this.  BOOST::program_options is great. The posix getopt is not.
 * 
 * What really motivated me to write Options was a desire to
 * eliminate boost dependencies from software that I've been developing.
 * One could use the BOOST program_options facility. It is very flexible,
 * a model of spectacular use of templates.  I am humbled everytime I
 * look at it.   But carrying boost around is like bringing a piano on
 * a picnic -- for some things it is the only tool to use, but for
 * the most part it gets pretty heavy when you have to haul it home
 * after all that fried chicken, potato salad, and lemonade. 
 *
 * If this looks a lot like boost::program_options, then that is no
 * accident.  But I just need to get rid of this piano.
 * 
 * ## Enough of the witty and entertaining chit-chat. Let's look at an example.
 *
 * The snippets below are from the OptionsExample.cxx program in the example directory. 
 * 
 * The Options object is a command line parser that scans an argc/argv list
 * for options (thingies that start with -- or -), their values, and positional
 * arguments. The application programmer specifies the legal option names, 
 * their argument types, and a location for the scanned value -- if found. 
 * 
 * The parser is primitive -- don't expect the spectacular and sophisticated 
 * grammar of getopt or boost::program_options.  The intent here is bare-bones, 
 * just enough to satisfy the simplest needs.  If your needs are beyond that, 
 * then get the trailer ready to drag that piano to the picnic. 
 * 
 * So, let's assume we want to parse a command line that looks something like
 * this: 
 * \verbatim
$ ./OptionsExample -i 3 --presarg  --boolarg 1 fred --strvecarg one --strvecarg "two two" --strvecarg 3  john paul george ringo
\endverbatim
 * Which would print
 * \verbatim
intarg = 3
boolarg = 1
pres_arg = 1
str_arg = []
strvecarg s = 
	[one]
	[two two]
	[3]
An intarg option was present
posargs = 
	0	fred
	1	john
	2	paul
	3	george
	4	ringo
\endverbatim
 *
 * This is how we might build that: 
 * \snippet OptionsExample.cxx describe the command line
 * 
 * The example creates a Options object, and then decorates it with options. 
 * There are three kinds of options:
 * 
 * * Presence options, added with Options::addP.  A presence option
 * sets a selected bool value if it appears on the command line. It takes no
 * value on the command line.

 * * Scalar options, added with Options::add<T>. Scalar options take one argument on
 * the command line. A scalar option can be of any type that is capable of
 * being read by an input stream. Typically, these are int, float, 
 * double, unsigned int, long, unsigned long, or string. If a value is 
 * not supplied, the default value is "filled in" to the target value. 
 * 
 * * Vector options, added with Options::addV<T>. These are identical to scalar 
 * options, with the exception that a vector option may be specified multiple
 * times.  The values taken from the command stream are appended to the
 * supplied vector. 
 * 
 * Help and other command information can be added with the Options::addInfo method. 
 * 
 * Once the options have been added, and any info supplied, the application
 * can parse the command line by calling Options::parse. Like this
 * 
 * \snippet OptionsExample.cxx parse it
 * 
 * Can't get much simpler than that.  
 *
 * Each of the supplied values will be filled in.  The addP target will be
 * set to "true" if the option appeared on the command line. The app
 * can determine if a particular option appeared at all by calling 
 * Options.isPresent with the long name of the option. 
 */

  /**
   * @brief Exception complains about an option that isn't recognized. 
   * 
   * @param tkn the option string (long or abbreviated) that was not found. 
   */
  class BadOptionNameException : public std::runtime_error {
  public:
    BadOptionNameException(const std::string & tkn) : std::runtime_error("Command option [[" + tkn + "]] is unknown.") {
    } 
  }; 

  /**
   * @brief Complain if the supplied value (string) can't be parsed
   *
   * Each option defined by a call to Options::add or Options::addV
   * is associated with a type.  If the string supplied for the option's value
   * can't be converted to the associated type, the parser will throw this 
   * exception. 
   */
  class BadOptValueException : public std::runtime_error {
  public:
    BadOptValueException(const std::string & long_name,
		const std::string & badstr, 
		const std::string & err_msg) : 
      runtime_error(long_name + " unacceptable value \"" + 
		    badstr + "\" " + err_msg) {
    }
  }; 

  /**
   * @brief Command line parser class. 
   * 
   */
  class Options : public UtilsBase {
  public:
    /**
     * @brief constructor.  
     * 
     * @param is_kvp If true, this parser is for a key=value pair list. 
     * (see parseKeyValue)
     */
    Options(bool is_kvp = false);

    /**
     * @brief add informative information to the printHelp message. 
     * Multiple calls will append each string to the end of the information
     * list. 
     * 
     * @param info explanation
     */
    Options & addInfo(const std::string & info);

    /**
     * @brief default argument checker.  Returns true regardless of 
     * the value of its parameter. 
     *
     * @param v to be ignored. 
     * @return true
     */
    template <typename T>
    static bool allGood(T v) { return true; }
    
    /**
     * @brief Add an option specifier to the command line that sets
     * a target variable to the argument for the option. 
     * 
     * Each option is specified by a long name (e.g. "--set")
     * and a short name (e.g. "-s").  Each option will set a 
     * variable specified by the caller.  An argument may be
     * a std::string, or any type that can be read from a stream.
     * 
     * @param val pointer to variable that will be set if/when 
     * this option is found on a command line. 
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param def_val the default value for *val
     * @param doc_str describes the meaning of the option
     * @param test_func returns true if the value supplied on the command 
     * line is acceptable. 
     * @param err_msg message to be printed if the argument value is unacceptable
     * 
     * @return Options object
     */
    template <typename T>
    Options & add(T * val,
		  const std::string & long_name,
		  char ab_name, 
		  T def_val,
		  const std::string & doc_str = std::string(""),
		  const std::function<bool(T)> & test_func = [](T v){ return true; },
		  const std::string & err_msg = std::string("")) {

      // create an arg object and push it. 
      OptBase_p arg_p = std::make_shared<Opt<T>>(val, 
						 def_val,
						 std::is_signed<T>::value,
						 doc_str, test_func, err_msg);

      registerOpt(arg_p, long_name, ab_name);
      return *this;      
    }

    template <typename T>
    Options & add(T * val,
		  const std::string & long_name,
		  char ab_name, 
		  const std::string & doc_str = std::string(""),
		  const std::function<bool(T)> & test_func = [](T v){ return true; },
		  const std::string & err_msg = std::string("")) {

      // create an arg object and push it. 
      OptBase_p arg_p = std::make_shared<Opt<T>>(val, std::is_signed<T>::value,
						 doc_str, test_func, err_msg);

      registerOpt(arg_p, long_name, ab_name);
      return *this;      
    }
    
    /**
     * @brief Add an option specifier to the command line that takes no
     * argument, but may be tested for its "presence" on the command line. 
     * 
     * 
     * @param val pointer to variable that will be set if/when 
     * this option is found on a command line. 
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param doc_str describes the meaning of the option
     * 
     * @return Options object
     */
    Options & addP(bool * val,
		   const std::string & long_name, 
		   char ab_name, 
		   const std::string & doc_str = std::string("")) {
      OptBase_p arg_p = std::make_shared<OptPresent>(val, doc_str);
      registerOpt(arg_p, long_name, ab_name);
      return *this;      
    }
		   
    /**
     * @brief Add an option specifier to the command line that sets
     * a target variable to the argument for the option. Multiple 
     * instances of the option may be specified. The supplied value
     * is pushed to the end of the target variable. 
     * 
     * Each option is specified by a long name (e.g. "--set")
     * and a short name (e.g. "-s").  Each option will set a 
     * variable specified by the caller.  An argument may be
     * a std::string, or any type that can be read from a stream.
     * 
     * @param val pointer to variable that will be set if/when 
     * this option is found on a command line. 
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param doc_str describes the meaning of the option
     * @param test_func returns true if the value supplied on the command 
     * line is acceptable. 
     * @param err_msg message to be printed if the argument value is unacceptable
     * 
     * @return Options object
     */
    template <typename T>
    Options & addV(std::vector<T> * val,
		   const std::string & long_name, 
		   char ab_name, 
		   const std::string & doc_str = std::string(""),
		   const std::function<bool(T)> & test_func = [](T val){ return true; },
		   const std::string & err_msg = std::string("")) {

      OptBase_p arg_p = std::make_shared<OptVec<T>>(val, doc_str, test_func, err_msg);
      registerOpt(arg_p, long_name, ab_name);
      
      return *this;
    }
		   
		   
    /**
     * @brief Parse the command line. 
     * 
     * Scan the argv and set any target variables
     * 
     * @param argc count of token on the line (including the program name)
     * @param argv pointers to each token
     * 
     * @return true if there was no problem interpreting the command line. 
     * false on error. 
     *
     */
    bool parse(int argc, char * argv[]);

    /**
     * @brief Parse a list of tokens.
     * 
     * @param arglist list of tokens to be parsed. 
     * 
     * @return true if there was no problem interpreting the list.
     * false on error. 
     *
     */
    bool parse(std::list<std::string> arglist);

    /**
     * @brief Parse a list of tokens from a string
     * 
     * @param s string of tokens to be parsed. 
     * 
     * @return true if there was no problem interpreting the list.
     * false on error. 
     *
     */
    bool parse(const std::string & s);
    
    /**
     * @brief print the help and info strings. 
     * 
     * @param ostr output stream
     * @return reference to the output stream. 
     *
     */
    std::ostream & printHelp(std::ostream & ostr); 

    /**
     * @brief test for appearance of an option. 
     * 
     * @param long_name long name of the option
     * @return true if the option appeared on the command line as either long_name or ab_name.
     */
    bool isPresent(const std::string & long_name);

    /**
     * @brief test for appearance of an option. 
     * 
     * @param ab_name short (abbreviated, one character) name of the option
     * @return true if the option appeared on the command line as either long_name or ab_name.
     */
    bool isPresent(char ab_name);

    /**
     * @brief return a vector of the positional arguments. 
     *
     * @return a reference to a const vector containing the positional arguments 
     */
    const std::vector<std::string> & getPosArgs();


    /**
     * @brief return nth positional argument
     *
     * Tokens on the command line that are not apparently values to be 
     * supplied to an option are appended to a vector of positional arguments.
     * 
     * @param idx The position of the positional argument in the command line. 
     * @return the idx'th token in the positional list
     */
    std::string getPosArg(int idx);

    /**
     * @brief return the number of positional arguments
     * 
     * @return the number of positional arguments. 
     */
    int numPosArgs() { return pos_arg_vec.size(); }

    /**
     * @brief parse argument list in the form of a key-value pair
     * 
     * Used for options-lists within options-lists like
     * ```
     * my_program --internal_args "infile=foo, outfile=bar"
     * ```
     * since lists-within-lists don't really work all that well. 
     * 
     * Indvidual key-value pairs should be separated by commas
     * 
     * @param s string of key=value pairs to be parsed
     * 
     * @return true if there was no problem interpreting the list
     * false on error. 
     * 
     */
    bool parseKeyValue(const std::string & s);

/**
     * @brief parse argument list in the form of a key-value pair
     * 
     * Used for options-lists within options-lists like
     * ```
     * my_program --internal_args "infile=foo, outfile=bar"
     * ```
     * since lists-within-lists don't really work all that well. 
     * 
     * Indvidual key-value pairs should be separated by commas
     * 
     * @param ls a list of a string of key=value pairs to be parsed
     * 
     * @return true if there was no problem interpreting the list
     * false on error. 
     * 
     */
    bool parseKeyValue(const std::list<std::string> & ls);
    
  private:

    int isSwitch(const std::string & tkn);

    class OptBase; 
    typedef std::shared_ptr<OptBase> OptBase_p; 
    
    class OptBase {
    public:
      OptBase(const std::string & doc_str, 
	      const std::string & err_msg, 
	      bool is_signed, 
	      bool has_default = true) : 
	doc_str(doc_str), err_msg(err_msg), 
	is_signed(is_signed), has_default(has_default) {
	present = false; 
      }

      std::ostream & printHelp(std::ostream & os);

      bool isPresent() {
	return present; 
      }

      virtual bool isPresentOpt() { 
	return false; }

      virtual bool setVal(const std::string & vstr) = 0;

      virtual bool setPresent() {
	bool old = present; 
	present = true; 
	return old; 
      }
    
      void setNames(const std::string & ln, char abn) {
	long_name = ln;
	ab_name =abn; 
      }
      
      virtual bool hasDefault() { return has_default; }
      
      std::string long_name;
      char ab_name;
      bool has_default;
      bool is_signed; 
      
    protected:
      
      template<typename T> 
      void setValBase(T & v, const std::string & vstr) {
	std::stringstream ss(vstr, std::ios::in); 
	ss >> v;
	if(!ss) {
	  throw BadOptValueException(long_name, vstr, err_msg); 
	}
      }

      void setValBase(std::string & v, const std::string & vstr) {
	v = vstr; 
      }

      void setValBase(bool & v, const std::string & vstr) {
	auto vs = vstr;
	std::transform(vs.begin(), vs.end(), vs.begin(), 
		       [](unsigned char c){ return std::toupper(c);} );

	if(vs.size() == 0) {
	  v = false;
	}
	if((vs == "TRUE") || (vs[0] == 'T')) {
	  v = true;
	}
	else if((vs == "FALSE") || (vs[0] == 'F')){
	  v = false;
	}
	else {
	  int foo;
	  std::stringstream ss(vs, std::ios::in);
	  ss >> foo;
	  if(!ss) {
	    throw BadOptValueException(long_name, vstr, err_msg); 	    
	  }
	  else {
	    v = (foo != 0);
	  }
	}
	return; 
      }

      std::string doc_str;
      std::string err_msg;
      
      bool present; 
    }; 

    template <typename T> 
    class Opt : public OptBase {
    public:
      Opt(T * val,
	  T def_val,
	  bool is_signed, 
	  const std::string & doc_str = std::string(""),
	  const std::function<bool(T)> & test_func = allGood,
	  const std::string & err_msg = std::string("")) : 
	OptBase(doc_str, err_msg, is_signed, true), val_p(val), test_func(test_func)
      {
	*val = def_val; 
      }

      Opt(T * val,
	  bool is_signed, 	  
	  const std::string & doc_str = std::string(""),
	  const std::function<bool(T)> & test_func = allGood,
	  const std::string & err_msg = std::string("")) : 
	OptBase(doc_str, err_msg, is_signed, false), val_p(val), test_func(test_func)
      {
	*val = T();
      }
      
      bool setVal(const std::string & vstr) {
	setValBase(*val_p, vstr);
	if (!test_func(*val_p)) {
	  throw BadOptValueException(long_name, vstr, err_msg); 	  
	}
	return setPresent(); 
      }

    protected: 
      T * val_p;
      std::function<bool(T)> test_func; 
    };


    template <typename T>
    class OptVec : public OptBase {
    public:
      OptVec(std::vector<T> * v_vec,
	     const std::string & doc_str, 
	     const std::function<bool(T)> & test_func = allGood, 
	     const std::string & err_msg = std::string("")) :
	OptBase(doc_str, err_msg, false), 
	argvec_p(v_vec), 
	test_func(test_func)
      {
      }

      bool setVal(const std::string & vstr) {
	T v;
	setValBase(v, vstr);

	if (!test_func(v)) {	
	  throw BadOptValueException(long_name, vstr, err_msg);
	}

	argvec_p->push_back(v);

	present = true; 	

	return true; 
      }

      std::vector<T> * argvec_p;
      std::function<bool(T)> test_func;       
    };

    class OptPresent : public OptBase {
    public:
      OptPresent(bool * val,
		 const std::string & doc_str = std::string(""))
	: OptBase(doc_str, "", false) {
	*val = false; 
	val_p = val; 
      }

      bool isPresentOpt() { 
	return true; 
      }

      bool setVal(const std::string & vstr) { 
	*val_p = true; 
	present = true; 
	return true; 
      }

      bool setPresent() {
	*val_p = true;
	return OptBase::setPresent();
      }
      
      bool * val_p; 
    };

    void registerOpt(OptBase_p arg_p, 
		  const std::string & long_name, 
		  char ab_name); 

    OptBase_p findOpt(char c);

    OptBase_p findOpt(const std::string & key);


    std::list<std::string> buildTokenList(int argc, char * argv[]);
    std::list<std::string> buildTokenList(const std::string & s);    
    
    std::map<std::string, OptBase_p > long_map;
    std::map<char, OptBase_p > ab_map; 

    std::list<std::string> info_list; 
    
    std::vector<std::string> pos_arg_vec;

    bool is_kvp; 
    bool waiting_for_signed; 
  };
  
}
