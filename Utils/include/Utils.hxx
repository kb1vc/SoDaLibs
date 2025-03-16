#pragma once
/*
BSD 2-Clause License

Copyright (c) 2021, Matt Reilly - kb1vc
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
 * @file Utils.hxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 12, 2021
 */


#include <string>
#include <list>
#include <vector>

namespace SoDa {

/**
 * @page Utils Utils: Useful functions and such that fill in some gaps
 * 
 * Utils is a basket of little classes, functions, definitions, and
 * such that aren't normally provided in C++11 and its standard libraries. 
 * Eventually most of this stuff will disappear, as something like it (though
 * not identical) will get absorbed into the standard. 
 * 
 * Many of these features can be found in boost.  But I've been trying to
 * get away from boost dependencies in the various SoDa tools. That may
 * be folly, but what the heck. 
 * 
 */


  /**
   * @brief squash sequences of spaces into a single space. 
   * 
   *
   * 
   * @param str input string -- things like "foo  bar" will be collapsed to "foo bar"
   * @returns the good version
   */
  std::string squashSpaces(const std::string & str);
  
  /**
   * @brief split a string into a list of strings, based on a set 
   * of delimiter characters. 
   *
   *
   * 
   * @param str the string to be chopped up into tokens
   * @param delims a list of delimiter characters
   * @param no_empty if true, empty tokens will not be saved 
   * (e.g. foo,,bar will produce {foo,bar} not {"foo", "", "bar"})
   * @returns a list of tokens split by the delimiters. 
   */
  std::list<std::string> split(const std::string & str, const std::string delims, bool no_empty = false);

  /**
   * @brief split a string into a vector of strings, based on a set 
   * of delimiter characters. 
   *
   *
   * 
   * @param str the string to be chopped up into tokens
   * @param delims a list of delimiter characters
   * @param no_empty if true, empty tokens will not be saved 
   * (e.g. foo,,bar will produce {foo,bar} not {"foo", "", "bar"})
   * @returns a vector of tokens split by the delimiters. 
   */
  std::vector<std::string> splitVec(const std::string & str, const std::string delims, bool no_empty = false);

}
