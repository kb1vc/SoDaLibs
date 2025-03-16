#pragma once

/*
  BSD 2-Clause License

  Copyright (c) 2022, Matt Reilly - kb1vc
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
 * @file NoCopy.hxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 21, 2022
 */

/**
 * @page SoDa::NoCopy NoCopy: A base class for non-copyable classes
 *
 *
 * Certain objects contain state that is "shared" or visible to
 * multiple threads. If copied, the state in the new copy will become
 * stale and irrelevant. (Imagine a copy of a barrier. What will a
 * thread wait on in the copied barrier? Yes there are ways to fix
 * this, but jeeze, let's keep it simple for once. 
 *
 */

/**
 * @namespace SoDa
 * 
 * Not much else is spelled that way, so we're probably not going to
 * have too many collisions with code written by other people.
 *
 * SoDa is the namespace I use in code like
 * <a href="https://kb1vc.github.io/SoDaRadio/">SoDaRadio</a> (The S
 * D and R stand for Software Defined Radio.  The o,a,d,i,o don't
 * stand for anything in particular.)  But this code has nothing to do
 * with software defined radios or any of that stuff.
 */
namespace SoDa {

  
  
  /**
   * @class NoCopy
   *
   * @brief This is a base class that prevents derived classes from
   * using the assumed copy or assignment constructors.
   *
   */

  class NoCopy {
  public:
    // normal constructor doesn't do anything
    NoCopy() { }
    
    // copy/assign constructors don't exist. So no subordinate
    // class can do that stuff either. 
    NoCopy(const NoCopy& other) = delete;
    NoCopy & operator=(NoCopy & other) = delete;
  };
  
}



    
