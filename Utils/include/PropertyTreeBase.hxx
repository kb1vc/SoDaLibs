#pragma once
/*
BSD 2-Clause License

Copyright (c) 2022, 2025 Matt Reilly - kb1vc
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

#include <vector>
#include "PropertyException.hxx"
#include "PropertyNode.hxx"
#include "PropertyPath.hxx"

/**
 * @file PropertyTree.hxx
 * @author Matt Reilly (kb1vc)
 * @date April 12, 2025
 */

/**
 * @page SoDa::PropertyTree PropertyTree A simple property tree widget with
 * documentation. 
 * 
 * This is meant to be subclassed for file formats like XML and JSON. The
 * specialization only needs to supply a readFile and writeFile method.
 *
 * So, here is SoDa::PropertyTree -- 
 */

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
namespace SoDa::Property {
  
    /**
     * @class PropertyTree 
     * @brief Defines the tree and insert/retrieve functions for all derived trees. 
     */
  
  class Tree {
  public:
    /**
     * @brief The constructor.
     *
     * Create a property tree.  There won't be anything in it. 
     */
    Tree();

    /**
     * @brief Read a property file
     *
     * Add the stuff in "filename" to the property tree in this
     * object. 
     *
     * @param filename the name of the property file to be read
     * @throws SoDa::FileNotFound (a subclass of SoDa::Exception)
     * @throws SoDa::Property::BadSyntax
     */
    virtual void readFile(const std::string & filename);

    /**
     * @brief Read a property file
     *
     * Add the stuff in input stream to the property tree in this
     * object. 
     *
     * @param instream Input stream containing the property tree text to be read
     * @throws SoDa::Property::BadSyntax
     */
    virtual void readFile(std::istream & filename) = 0;
    
    /**
     * @brief Write a property file
     *
     * Write the stuff in "filename" to the property tree in this
     * object. 
     *
     * @param filename the name of the file to be written
     * @throws SoDa::NoFileCreated (a subclass of SoDa::Exception)     
     * @throws SoDa::Property::BadSyntax
     */
    virtual void writeFile(const std::string & filename);

    /**
     * @brief Write a property file
     *
     * Write the stuff to the output stream from the property tree in this
     * object. 
     *
     * @param filename the name of the file to be written
     */
    virtual void writeFile(std::ostream & filename);
    
    /*
     * @brief Return subpath 
     */
    
    /**
     * @brief dump the tree in a somewhat readable format. 
     *
     */
    std::ostream & dump(std::ostream & os); 
    
    /**
     * @brief get the root node.
     *
     * @return a pointer to the root node
     *
     */
    Node * getRoot() { return root; }
    
    /**
     * @brief get an attribute from the tree
     * @param v the value to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param throw_exception throws an exception if the pathname is not
     * found or if the type of the value cannot be read from the property
     * string at that node.
     * @returns true if the value was found and was readable from the property string. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     * @throws SoDa::PropertyTree::BadPropertyType
     */
    template<typename T> bool get(T & v, 
				  const std::string & pathname, 
				  bool throw_exception = false) {
      PropNode * pn = root->getProp(pathname, throw_exception); 
      
      if(pn == nullptr) {
	throw PropNode::PropertyNotFound(pathname); 
      }
      
      return pn->get<T>(v, throw_exception);
    }


    /**
     * @brief put an attribute into the tree (possibly replace a value)
     *
     * @param v the value to be written to the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     *
     * @returns true if the attribute was found, false if it was created.
     *
     */
    template<typename T> bool put(const T & v, 
				  const std::string & pathname)
    }
  public:
    


  public:

  protected:
    PropNodePtr root; 
  };
    
}



