#pragma once
#include "UtilsBase.hxx"
#include <memory>
/*
BSD 2-Clause License

Copyright (c) 2021, 2022, 2025 Matt Reilly - kb1vc
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
 * @date April 5, 2025
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
 * @page SoDa::Options Options: A simple command line parser
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
 *
 * \section SafeOptions Safe Clean Modern Options
 *
 * The first several versions of SoDa::Options used a scheme that is
 * terrifying in retrospect. If you are stuck dealing with some older
 * code, you can see how all that worked down below in \ref
 * ObsoleteOptions "Obsolete Interface" The interface was based on the
 * Boost::program_options class. That was a bad choice.
 *
 * The Options object stores *pointers* to variables that the application
 * wishes to be set from the command line. Then each pointer and the storage it points to
 * is created by the cmd object, so the pointer won't become invalid until the cmd object
 * goes out of scope.
 *
 * So let's look at how a command line object is created
 *
 * \snippet SafeOptionsExample.cxx SO describe the command line
 *
 * Note that the ```add``` and ```addP``` methods all return a shared pointer.
 * The example uses "auto" here, because an explicit definition doesn't really add
 * anything to the code, does it?
 *
 * The Options object can parse the standard ```argc``` ```argv``` stuff. Before Options::parse
 * is called, all values are set to their defaults, and ```isPresent``` will return false for all
 * option strings.
 *
 * \snippet SafeOptionsExample.cxx SO parse it
 *
 * The values are all accessed through the pointers returned by ```add``` and ```addP```
 *
 * \snippet SafeOptionsExample.cxx SO print the values
 *
 * The test for presence is pretty straightforward.
 *
 * \snippet SafeOptionsExample.cxx SO test for presence
 *
 * Positional arguments are returned as a vector of objects (not references)
 * The vector will outlive the Options object. 
 *
 * \snippet SafeOptionsExample.cxx SO get positional arguments
 *
 * The Options object may be re-used. A call to ```reset``` sets the value
 * of all optional parameters to their default. It also clears the posarg list.
 *
 * \snippet SafeOptionsExample.cxx SO reset
 *
 * SoDa::Options can also parse a string, like this:
 *
 * \snippet SafeOptionsExample.cxx SO string input
 *
 * So that's it.  Use this new safer interface to the SoDa::Options object.
 * 
 * 
 * \section ObsoleteOptions Caution: The scheme described here is OBSOLETE, but remains supported.
 *
 * The original options code counted on the validity of all pointers passed into the ```add``` methods.
 * This has the obvious liability that a call to "parse" could reference an invalid pointer.
 * New code should use the add methods that return a shared pointer to an appropriate type. this is
 * described in Section \ref SafeOptions "Safe Options Interface."
 * 
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
     * This form is OBSOLETE See \ref SafeOptions "Safe Options" done the right way.
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
		  const T def_val = T(),
		  const std::string & doc_str = std::string(""),
		  const std::function<bool(T)> & test_func = [](T v){ return true; },
		  const std::string & err_msg = std::string("")) {

      // create an arg object and push it.
      OptBase_p arg_p = std::make_shared<Opt<T>>(val,
						 std::is_signed<T>::value,
						 def_val,
						 doc_str, test_func, err_msg);

      *val = def_val; 
      registerOpt(arg_p, long_name, ab_name);
      return *this;      
    }

    
    /**
     * @brief Add an option specifier to the command line that sets
     * a target variable to the argument for the option. 
     * 
     * Each option is specified by a long name (e.g. "--set")
     * and a short name (e.g. "-s").  Each option will set a 
     * variable specified by the caller.  An argument may be
     * a std::string, or any type that can be read from a stream.
     * 
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param def_val the default value for *val
     * @param doc_str describes the meaning of the option
     * @param test_func returns true if the value supplied on the command 
     * line is acceptable. 
     * @param err_msg message to be printed if the argument value is unacceptable
     * 
     * @return shared pointer to a value of type T that may be referenced at any time. 
     */
    template <typename T>
    std::shared_ptr<T> add(const std::string & long_name,
			   char ab_name,
			   T def_val = T(),
			   const std::string & doc_str = std::string(""),
			   const std::function<bool(T)> & test_func = [](T v){ return true; },
			   const std::string & err_msg = std::string("")) {
      
      // create a pointer to an object of type T and initialize it with the default value
      auto val_ptr = std::make_shared<T>(def_val);
	
      // create an arg object and push it.	
      OptBase_p arg_p = std::make_shared<Opt<T>>(val_ptr,
						 std::is_signed<T>::value,
						 def_val,
						 doc_str, test_func, err_msg);

      *val_ptr = def_val; 
      registerOpt(arg_p, long_name, ab_name);
      return val_ptr;
    }

    
    /**
     * @brief Add an option specifier to the command line that takes no
     * argument, but may be tested for its "presence" on the command line. 
     * 
     * 
     * @param present_p pointer to variable that will be set if/when 
     * this option is found on a command line. 
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param doc_str describes the meaning of the option
     * 
     * @return Options object
     */
    Options & addP(bool * present_p, 
		   const std::string & long_name, 
		   char ab_name, 
		   const std::string & doc_str = std::string("")) {
      auto val_ptr = std::make_shared<bool>(false);
      OptBase_p arg_p = std::make_shared<OptPresent>(present_p, doc_str);
      registerOpt(arg_p, long_name, ab_name);
      return *this;
    }

    /**
     * @brief Add an option specifier to the command line that takes no
     * argument, but may be tested for its "presence" on the command line. 
     * 
     * 
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param doc_str describes the meaning of the option
     * 
     * @return present_p shared pointer to variable that will be set if/when 
     * this option is found on a command line. 
     */
    std::shared_ptr<bool> addP(const std::string & long_name, 
			       char ab_name, 
			       const std::string & doc_str = std::string("")) {
      auto val_ptr = std::make_shared<bool>(false);
      OptBase_p arg_p = std::make_shared<OptPresent>(val_ptr, doc_str);
      registerOpt(arg_p, long_name, ab_name);
      return val_ptr; 
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
     * @param vec_ptr pointer to variable that will be set if/when 
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
    Options & addV(std::vector<T> * vec_ptr, 
		   const std::string & long_name, 
		   char ab_name, 
		   const std::string & doc_str = std::string(""),
		   const std::function<bool(T)> & test_func = [](T val){ return true; },
		   const std::string & err_msg = std::string("")) {
      
      OptBase_p arg_p = std::make_shared<OptVec<T>>(vec_ptr, doc_str, test_func, err_msg);
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
     * @param long_name the long version of the option name 
     * @param ab_name the short (one character) version of the option name
     * @param doc_str describes the meaning of the option
     * @param test_func returns true if the value supplied on the command 
     * line is acceptable. 
     * @param err_msg message to be printed if the argument value is unacceptable
     * 
     * @return val pointer to a list that will accumulate values from the command line.
     */
    template <typename T>
    std::shared_ptr<std::vector<T>> addV(const std::string & long_name, 
					 char ab_name, 
					 const std::string & doc_str = std::string(""),
					 const std::function<bool(T)> & test_func = [](T val){ return true; },
					 const std::string & err_msg = std::string("")) {

      auto val_ptr = std::make_shared<std::vector<T>>();

      OptBase_p arg_p = std::make_shared<OptVec<T>>(val_ptr, doc_str, test_func, err_msg);
      registerOpt(arg_p, long_name, ab_name);
      
      return val_ptr; 
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
     * @brief reset the object
     *
     *
     */
    void reset();
    
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
     * A vector is returned, rather than a reference, so that the
     * vector's values outlive the cmd object, and outlive a call to reset.
     *
     * @return a reference to a  vector containing the positional arguments 
     */
    std::vector<std::string> getPosArgs();


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

      virtual void reset() { }
      
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
      Opt(T * u_val_ptr,
	  bool is_signed,
	  T def_val,
	  const std::string & doc_str = std::string(""),
	  const std::function<bool(T)> & test_func = allGood,
	  const std::string & err_msg = std::string("")) : 
	OptBase(doc_str, err_msg, is_signed, false), 
	def_val(def_val),
	u_val_ptr(u_val_ptr), 
	test_func(test_func)
      {
	s_val_ptr = nullptr; 
      }

      
      Opt(std::shared_ptr<T> s_val_ptr,
	  bool is_signed,
	  T def_val,
	  const std::string & doc_str = std::string(""),
	  const std::function<bool(T)> & test_func = allGood,
	  const std::string & err_msg = std::string("")) : 
	OptBase(doc_str, err_msg, is_signed, false), 
	s_val_ptr(s_val_ptr), 
	test_func(test_func), 
	def_val(def_val)
      {
	u_val_ptr = nullptr;

      }

      bool setVal(const std::string & vstr) {
	T new_val;
	setValBase(new_val, vstr);
	
	if(u_val_ptr != nullptr) {
	  *u_val_ptr = new_val; 
	}
	else {
	  *s_val_ptr = new_val; 
	}
	
	if (!test_func(new_val)) {
	  throw BadOptValueException(long_name, vstr, err_msg); 	  
	}
	return setPresent(); 
      }


      void reset() {
	if(s_val_ptr != nullptr) {
	  *s_val_ptr = def_val;
	}

	if(u_val_ptr != nullptr) {
	  *u_val_ptr = def_val; 
	}
      }
    protected: 
      T * u_val_ptr;
      T def_val;
      std::shared_ptr<T> s_val_ptr;
      std::function<bool(T)> test_func; 
    };


    template <typename T>
    class OptVec : public OptBase {
    public:
      OptVec(std::vector<T> * v_vec_ptr,
	     const std::string & doc_str, 
	     const std::function<bool(T)> & test_func = allGood, 
	     const std::string & err_msg = std::string("")) :
	OptBase(doc_str, err_msg, false), 
	u_arg_vec_ptr(v_vec_ptr), 
	test_func(test_func)
      {
	s_arg_vec_ptr = nullptr; 
      }

      OptVec(std::shared_ptr<std::vector<T>> v_vec_ptr,
	     const std::string & doc_str, 
	     const std::function<bool(T)> & test_func = allGood, 
	     const std::string & err_msg = std::string("")) :
	OptBase(doc_str, err_msg, false), 
	s_arg_vec_ptr(v_vec_ptr), 
	test_func(test_func)
      {
	u_arg_vec_ptr = nullptr; 
      }
      
      bool setVal(const std::string & vstr) {
	T v;
	setValBase(v, vstr);

	if (!test_func(v)) {	
	  throw BadOptValueException(long_name, vstr, err_msg);
	}

	if(u_arg_vec_ptr != nullptr) {
	  u_arg_vec_ptr->push_back(v);
	}
	else if(s_arg_vec_ptr != nullptr) {
	  s_arg_vec_ptr->push_back(v); 
	}

	present = true; 	

	return true; 
      }

      void reset() {
	if(s_arg_vec_ptr != nullptr) {
	  s_arg_vec_ptr->clear();
	}
	if(u_arg_vec_ptr != nullptr) {
	  u_arg_vec_ptr->clear();
	}
      }

      std::shared_ptr<std::vector<T>> s_arg_vec_ptr;
      std::vector<T> * u_arg_vec_ptr; 
      std::function<bool(T)> test_func;       
    };

    class OptPresent : public OptBase {
    public:
      OptPresent(bool * val_p, 
		 const std::string & doc_str = std::string(""))
	: OptBase(doc_str, "", false), u_val_ptr(val_p) {
	s_val_ptr = nullptr; 
	*u_val_ptr = false; 
      }
      OptPresent(std::shared_ptr<bool> val_p, 
		 const std::string & doc_str = std::string(""))
	: OptBase(doc_str, "", false), s_val_ptr(val_p) {
	u_val_ptr = nullptr; 
	*s_val_ptr = false; 
      }
      
      bool isPresentOpt() { 
	if(s_val_ptr != nullptr) {
	  return *s_val_ptr; 
	}
	if(u_val_ptr != nullptr) {
	  return *u_val_ptr;
	}
	return false; 
      }

      bool setVal(const std::string & vstr) { 
	if(s_val_ptr != nullptr) {
	  *s_val_ptr = true; 
	}
	if(u_val_ptr != nullptr) {
	  *u_val_ptr = true; 
	}

	present = true; 
	return true; 
      }

      bool setPresent() {
	if(s_val_ptr != nullptr) {
	  *s_val_ptr = true; 
	}
	if(u_val_ptr != nullptr) {
	  *u_val_ptr = true; 
	}

	return OptBase::setPresent();
      }
      
      void reset() {
	if(s_val_ptr != nullptr) {
	  *s_val_ptr = false; 
	}
	if(u_val_ptr != nullptr) {
	  *u_val_ptr = false; 
	}
	present = false;

      }
      
      std::shared_ptr<bool> s_val_ptr; 
      bool * u_val_ptr; 
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
