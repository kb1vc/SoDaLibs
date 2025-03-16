#pragma once
#include <string>


/**
 * @mainpage SoDa:: Utilities
 * 
 * The sodautilities library provides some simple classes to do things like
 * parse command lines and format output in a way that a sensible person might
 * appreciate.
 * 
 * There are  pieces right now:
 *   - SoDa::Options -- a command line parser
 *   - SoDa::Format -- string formatting with a nice facility for "engineering notation"
 *   - SoDa::Barrier -- a simple interface to the Threads barrier stuff
 *   - SoDa::MailBox -- a publish/subscribe scheme for inter-thread communication
 *   - SoDa::Exception -- a wrapper for std::runtime_error that groups all SoDa exceptions (or is supposed to)
 *   - SoDa::NoCopy -- I have no idea why I created this. 
 *   - SoDa::PropertyTree -- three guesses as to what this is.  YAML is the only format supported right now.
 *   - SoDa::Utils -- string functions, and perhaps other "helpful" things
 *
 * @section Overview
 * 
 * The SoDa::Options and SoDa::Format classes are so cool that I'll describe them right here. 
 * 
 * @subsection OptionsSummary Options: a simple command line / option parser
 * 
 *  SoDa::Options is a class that allows the programmer to specify
 *  command line options (like --help, --out, --enable-deep-fry --set-sauce=Memphis)
 *  and parse the (argc, argv) input line.  There are other ways to do
 *  this.  BOOST::program_options is great. The posix getopt is not.
 *  
 *  What really motivated me to write SoDa::Options was a desire to
 *  eliminate boost dependencies from software that I've been developing.
 *  One could use the BOOST program_options facility. It is very flexible,
 *  a model of spectacular use of templates.  I am humbled everytime I
 *  look at it.   But carrying boost around is like bringing a piano on
 *  a picnic -- for some things it is the only tool to use, but for
 *  the most part it gets pretty heavy when you have to haul it home
 *  after all that fried chicken, potato salad, and lemonade. 
 *  
 * 
 *  If this looks a lot like boost::program_options, then that is no
 *  accident.  But I just need to get rid of that piano.
 * 
 * @subsection FormatSummary Format: print stuff.
 * 
 *  To get to doxygen generated documentation that is more detailed 
 *  than the following summary, go [here](https://kb1vc.github.io/SoDaFormat/)
 * 
 *  SoDa::Format is a class that allows intelligent formatting of
 *  integer, floating point, string, and character values into
 *  std::string objects or for output to a stream.  The concept is
 *  inspired by the much more capable Qt::QString's formatting
 *  features. If you can afford the library dependency, use Qt. It's
 *  quite good. 
 * 
 *  SoDa::Format is meant to improve upon the tremendously awkward, antiquated, and bizarre
 *  stream output features of the standard template library. (C++ stream IO was a giant step 
 *  backward from the comparatively flexible and intuitive FORTRAN "FORMAT" scheme. There isn't
 *  much in the computing world that is a giant step backward from FORTRAN circa 1975.)
 *  
 *  One could use the BOOST format facility. (One could also eat
 *  brussels sprouts for breakfast, but it isn't *my* kind of thing.
 *  Is it yours?) boost::format is tremendously powerful and fabulously
 *  documented.  It may be easily extended by beings living in some of
 *  the outer reaches of the Horse Head nebula. I just don't grok it
 *  myself. And carrying around a boost dependency is like growing an
 *  extra thumb -- it is useful at times, but it becomes less and less
 *  so as time goes on, and you can never quite remember why you
 *  got it in the first place.
 *  
 *  What really motivated me to write SoDa::Format was the lack of an
 *  "engineering notation" format for floating point numbers in any of
 *  the common formatting facilities.  As a dyed in the wool MKS
 *  engineer, this drives me right up the wall.  Exponents should be multiples of
 *  three.  If Adam had done any floating point arithmetic, he would have written
 *  it down in engineering notation.  (Perhaps he did, and then lost it all when
 *  the serpent screwed everything up.)  
 * 
 * So with SoDa::Format you can print out
 * 
 * 322.0e-6 Farads
 * 
 * instead of the grotesque
 * 
 * 3.22e-4 Farads.
 * 
 * Who even *thinks* like that?
 * 
 * @subsection Utilities
 * 
 * Right now the only "miscellaneous" stuff has to do with splitting strings.
 * See the documentation for Utils.hxx for more information. 
 * 
 *
 * @section License
 *
 * 
 * @copyright (c) 2020, 2021, 2025 Matt Reilly - kb1vc
 *
 * @par BSD 2-Clause License
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file UtilsBase.hxx
 * @author Matt Reilly (kb1vc)
 */

/**
 * @page SoDa::UtilsBase Utils: provide version info to all Utils classes
 */
namespace SoDa {
  class UtilsBase {
  public:
    UtilsBase() {
    }

    std::string getVersion();

    std::string getGitID();
  };
}
