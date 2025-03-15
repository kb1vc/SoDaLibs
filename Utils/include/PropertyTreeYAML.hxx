#pragma once
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include "Exception.hxx"
#include "PropertyTree.hxx"
/*
BSD 2-Clause License

Copyright (c) 2022 Matt Reilly - kb1vc
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
 * @file PropertyTreeYAML.hxx
 * @author Matt Reilly (kb1vc)
 * @date December 31, 2020
 */

/**
 * @page SoDa::PropertyTreeYAML A simple property tree widget built from a
 * YAML file.   
 *
 * This inherits the storage and retrieval methods from PropTree.
 * 
 * yaml-cpp is really pretty good, but the documentation is completely
 * inadequate. It also isn't very good at reporting errors, and if it
 * throws exceptions, I haven't found any documentation.
 *
 * So, here is SoDa::PropTreeYAML -- not as good as yaml-cpp but it is
 * documented somewhat. 
 */

/**
 * We do this here so that the end user doesn't need to include any
 * yaml headers, and in fact, yaml doesn't even need to be installed. 
 */
namespace YAML {
  class Node; 
}

/**
 * @namespace SoDa
 * 
 * Not much else is spelled that way, so we're probably not going to
 * have too many collisions with code written by other people.
 *
 * SoDa is the namespace I use in code like <a
 * href="https://kb1vc.github.io/SoDaRadio/">SoDaRadio</a> (The S D
 * and R stand for Software Defined Radio.  The o,a,d,i,o don't stand
 * for anything in particular.)  But this code has nothing to do with
 * software defined radios or any of that stuff.
 */
namespace SoDa {
  
    /**
     * @class PropertyTreeYAML
     * @brief Create a property tree from one or more YAML files. 
     */
  
  class PropertyTreeYAML : public PropertyTree {
  public:
    /**
     * @brief The constructor.
     *
     * Create a property tree.  There won't be anything in it. 
     */
    PropertyTreeYAML();

    /**
     * @brief The constructor.
     *
     * Create a property tree.  Fill it in with the YAML stuff in "filename"
     *
     * @param filename the name of the yaml file to be read
     * @throws SoDa::PropertyTree::FileNotFound
     * @throws SoDa::PropertyTreeYAML::BadSyntax
     */
    PropertyTreeYAML(const std::string & filename);

    /**
     * @brief Read a YAML file
     *
     * Add the YAML stuff in "filename" to the property tree in this
     * object. 
     *
     * @param filename the name of the yaml file to be read
     * @throws SoDa::PropertyTree::FileNotFound     
     * @throws SoDa::PropertyTreeYAML::BadSyntax
     */
    void readFile(const std::string & filename);

    /**
     * @brief Write a YAML file
     *
     * Traverse the property tree and write its contents
     * to a YAML file. 
     *
     * @param filename the name of the yaml file to be written
     * @throws SoDa::PropertyTree::FileNotFound
     */
    void writeFile(const std::string & filename); 
  protected: 
    void fillRecurse(const YAML::Node & node, 
		     PropertyTree::PropNode * prop);

  };
    
}



