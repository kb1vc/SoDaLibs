#pragma once
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <ios>
#include <map>
#include <list>
#include <typeinfo>
#include "Exception.hxx"
#include "Format.hxx"

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
 * @file PropertyTree.hxx
 * @author Matt Reilly (kb1vc)
 * @date December 31, 2020
 */

/**
 * @page SoDa::PropTree A simple property tree widget with
 * documentation. 
 * 
 * This is meant to be subclassed for file formats like XML and YAML.
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
namespace SoDa {
  
    /**
     * @class PropertyTree 
     * @brief Hold 
     */
  
  class PropertyTree {
  public:
    /**
     * @brief The constructor.
     *
     * Create a property tree.  There won't be anything in it. 
     */
    PropertyTree();



    /**
     * @brief Read a property file
     *
     * Add the stuff in "filename" to the property tree in this
     * object. 
     *
     * @param filename the name of the property file to be read
     * @throws SoDa::FileNotFound (a subclass of SoDa::Exception)
     * @throws SoDa::PropertyTree::BadSyntax
     */
    virtual void readFile(const std::string & filename) = 0;

    /**
     * @brief Write a property file
     *
     * Write the stuff in "filename" to the property tree in this
     * object. 
     *
     * @param filename the name of the file to be written
     * @throws SoDa::NoFileCreated (a subclass of SoDa::Exception)     
     * @throws SoDa::PropertyTree::BadSyntax
     */
    virtual void writeFile(const std::string & filename) = 0;

    /**
     * @brief dump the tree in a somewhat readable format. 
     *
     */
    std::ostream & dump(std::ostream & os); 
    
    
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
    
  public:
    class PropNode {
    public:
      // create a key-value pair, dictionary, list, or string (leaf)
      PropNode(PropNode * parent, 
	       const std::string & val_string, 
	       bool is_terminal); 

      PropNode * parent; 
      std::string val_string;
      bool is_terminal; 
      std::list<PropNode *> prop_list;
      std::map<std::string, PropNode *> dictionary;
      
    public:
      /**
       * @brief Translate the value string of this property
       * into a value of a specified type.
       *
       * @param v reference to the value we'll set
       * @param throw_exception if true we'll throw an exception when
       * bad things happen. 
       * @returns true if we found a valid value. false if the value
       * string did not contain a value of type "T"
       * @throws SoDa::PropertyTree::BadPropertyType
       */
      template<typename T> bool get(T & v, bool throw_exception = false) {
	std::stringstream ss(val_string, std::ios_base::in);
	ss >> v; 
	if(ss.fail()) {
	  if(throw_exception) {
	    throw PropertyTree::PropNode::BadPropertyType(getPathName(), typeid(v).name(), val_string);
	      }
	  else return false; 
	}
	return true; 
      }

      /**
       * @brief find the path name that led us to this node
       */
      std::string getPathName();
      
      /**
       * @brief get a property node from this node's dictionary
       * @param pathname the heirarchical path name to the attribute.
       * Levels in the tree are separated with ":" as in "top:next:leaf"
       * @param throw_exception throws an exception if the pathname is not
       * found.
       * @returns the property node, nullptr if not found
       * @throws SoDa::PropertyTree::PropertyNotFound
       * @throws SoDa::PropertyTree::NotADictionary
       */
      PropNode * getProp(const std::string & pathname, 
			 bool throw_exception = false);

      
      /**
       * @brief Get a list of properties from this node.
       *
       * @returns a list of all nodes in this node's prop_list
       * Returns an empty list if this node has no nodes in its nodelist
       * or is not a list.
       */
      std::list<PropNode *> getList(); 

      /**
       * @brief Get a list of keys from this node's dictionary.
       *
       * @returns a list of all key strings in this node's dictionary
       * Returns an empty list if this node is not a dictionary
       * or has no nodes in its dictionary.
       */
      std::list<std::string> getKeys(); 

      /**
       * @brief dump the tree in a somewhat readable format. 
       *
       */
      std::ostream & dump(std::ostream & os, std::string indent); 
      
     public: 
      

      class PropertyNotFound : public SoDa::Exception {
      public:	
	PropertyNotFound(const std::string & path); 
      }; 

      class BadPropertyType : public SoDa::Exception {
      public:	
	BadPropertyType(const std::string & path_name, 
			const std::string & type_name, 
			const std::string & val_string); 
      }; 
      
    protected: 
      /**
       * @brief get a property node from this node's dictionary
       * @param pathlist the original pathname split into 
       * parts by the ':' delimiter. 
       * @param orig_pathname the pathname before it got split up
       * @param throw_exception throws an exception if the pathname is not
       * found.
       * @returns the property node, nullptr if not found
       * @throws SoDa::PropertyTree::PropertyNotFound
       * @throws SoDa::PropertyTree::NotADictionary
       */
      PropNode * getPropRecursive(std::list<std::string> & pathlist, 
				  const std::string & orig_pathname, 
				  bool throw_exception = false);

      
    };

  public:
    class FileNotFound : public SoDa::Exception {
    public:
      FileNotFound(const std::string & fname);
    }; 

  protected:
    PropNode * root; 
  };
    
}



