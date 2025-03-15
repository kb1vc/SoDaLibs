# SoDa:: Utilities and Signal libraries

SoDa Utilities and SoDa Signals are libraries developed as part of the
SoDaRadio project that I've been working on for a long while. They
are C++ libraries. No attempt has been made to make them callable
from other languages. No attempt has been made to build them on
non-linux platforms.  

The sodautilities library provides some simple classes to do things
like parse com mand lines and format output in a way that a sensible
person might appreciate.

The sodasignals library provides classes that do the signal
processing required in SoDaRadio: FFT, filters, resamplers,
and periodograms.

To get to doxygen generated documentation that is more detailed than
the following summary, build the library, install it, and go to
{whateveryourinstallationrootis}/share/SoDaUtils/doc/html/index.html
or go to
{whateveryourinstallationrootis}/share/SoDaSignals/doc/html/index.html

Originally these two libraries were in separate github repos. But
SoDa Signals depends on SoDa Utils for string handling, and might
use more in the future. Bundling the two libraries into one
repo and, more importantly, one installation task simplifies
things a great deal. (Yes, git and cmake folks, I am sure there
is git and cmake magic that will do this without the repo merge.
But I couldn't make it work reliably.)
  
## SoDa:: Utilities

There are three pieces right now:
* Options -- a command line parser
* Format -- string formatting with a nice facility for "engineering notation"
* Utils -- string functions, and perhaps other "helpful" things

### SoDa::Options a simple command line / option parser

 SoDa::Options is a class that allows the programmer to specify
 command line options (like --help, --out, --enable-deep-fry --set-sauce=Memphis)
 and parse the (argc, argv) input line.  There are other ways to do
 this.  BOOST::program_options is great. The posix getopt is not.
 
 What really motivated me to write SoDa::Options was a desire to
 eliminate boost dependencies from software that I've been developing.
 One could use the BOOST program_options facility. It is very flexible,
 a model of spectacular use of templates.  I am humbled everytime I
 look at it.   But carrying boost around is like bringing a piano on
 a picnic -- for some things it is the only tool to use, but for
 the most part it gets pretty heavy when you have to haul it home
 after all that fried chicken, potato salad, and lemonade. 
 

 If this looks a lot like boost::program_options, then that is no
 accident.  But I just need to get rid of this piano.

### SoDa::Format print stuff.

 To get to doxygen generated documentation that is more detailed 
 than the following summary, go [here](https://kb1vc.github.io/SoDaFormat/)

 SoDa::Format is a class that allows intelligent formatting of
 integer, floating point, string, and character values into
 std::string objects or for output to a stream.  The concept is
 inspired by the much more capable Qt::QString's formatting
 features. If you can afford the library dependency, use Qt. It's
 quite good. 

 SoDa::Format is meant to improve upon the tremendously awkward, antiquated, and bizarre
 stream output features of the standard template library. (C++ stream IO was a giant step 
 backward from the comparatively flexible and intuitive FORTRAN "FORMAT" scheme. There isn't
 much in the computing world that is a giant step backward from FORTRAN circa 1975.)
 
 One could use the BOOST format facility. (One could also eat
 brussels sprouts for breakfast, but it isn't *my* kind of thing.
 Is it yours?) boost::format is tremendously powerful and fabulously
 documented.  It may be easily extended by beings living in some of
 the outer reaches of the Horse Head nebula. I just don't grok it
 myself. And carrying around a boost dependency is like growing an
 extra thumb -- it is useful at times, but it becomes less and less
 so as time goes on, and you can never quite remember why you
 got it in the first place.
 
 What really motivated me to write SoDa::Format was the lack of an
 "engineering notation" format for floating point numbers in any of
 the common formatting facilities.  As a dyed in the wool MKS
 engineer, this drives me right up the wall.  Exponents should be multiples of
 three.  If Adam had done any floating point arithmetic, he would have written
 it down in engineering notation.  (Perhaps he did, and then lost it all when
 the serpent screwed everything up.)  

So with SoDa::Format you can print out

322.0e-6 Farads

instead of the grotesque

3.22e-4 Farads.

Who even *thinks* like that?

### Utilities

Right now the only "miscellaneous" stuff has to do with splitting strings.
See the documentation for Utils.hxx for more information. 

## SoDa:: Signals


The sodasignals library provides classes that provide

* SoDa::FFT I like FFTW, but the API is strictly
  1980's FORTRAN. It needed a C++ interface.
* SoDa::Filter  The Filter class creates FIR filters
  with the window filter method. It also applies an
  apply method to run the filter on a single buffer. 
* SoDa::OSFilter Lots of textbooks show you how to
  build a filter for a single buffer. But applications
  that produce a stream of buffers to a filter require
  something like the Overlap and Save method. OSFilter
  does that.
* SoDa::Resampler  Resampling is hard. SoDa Signals
  tries to fix that for "reasonable" ratios that include
  all the ones that I've found important. (Like converting
  from 1.25 MS/s to 44.1 kS/s.)
* SoDa::Periodogram Spectral analysis is *not* DFT.
  The Periodogram class produces power spectral density
  vectors using one of a set of windows. 

See the doxygen documentation for a more detailed explanation. 

## Building the Libraries

I've tried to make this simple. 

### Dependencies

The dependencies are limited.
* git -- optional, but a good idea
* cmake
* C++ -- must support C++11
* FFTW -- without it the build will leave out SoDa:: Signals
* yaml-cpp -- need it for properties utility
* doxygen -- without it you don't get all the documentation
  that I worked so hard to write.

### Build Examples

I've built the libraries on "current" versions of Fedora and Ubuntu.
The scripts are part of Docker files found in the charliecloud
directory. The scripts install the "normal" programming tools
and FFTW.

Look there. 

### Installing

It is just like any other CMake project.  For instance, to install the
package in /usr/local ... From this directory

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ../
make
sudo make install
```


This will install the libraries in /usr/local/lib or lib64 as appropriate
and all the useful include files in /usr/local/include/SoDa

It will also write doxygen output that starts at /usr/local/share/doc/SoDaUtils/html/index.html and
/usr/local/share/doc/SoDaSignals/html/index.html and 


#### Installing Without root
It is possible to install the library in a private directory (without needing root) like this: 


```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${HOME}/my_tools/ ../
make
sudo make install
```

This should work just fine, but if you do, then any build that *uses*
SoDaSignals needs to add this to its cmake

```
cmake -DCMAKE_PREFIX_PATH=${HOME}/my_tools ../
```

That will tell cmake to look in your directory for the relevant cmake
files that describe where to find the libraries and headers.

### Testing and Using it all

Take a look at the Utils/Examples and Signals/Examples directories.

Take a look at the CMakeLists.txt files to see how to refer
to the SoDa libraries. 

If the installation went well, you can copy them from wherever your
distro puts Examples.
Then you should be able to do this from this directory.  But remember, if you installed the utils in some nonstandard directory, you'll need to add
```
cmake -DCMAKE_PREFIX_PATH=${HOME}/my_tools ../
```

```
cd example
mkdir build
cd build
cmake ../
make
```

Did that work?

