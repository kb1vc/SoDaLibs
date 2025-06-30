#pragma once
#include <string>
#include <memory>
#include <json/json.h>
#include "Exception.hxx"


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

/**
 * @file PropertyTree.hxx
 * @author Matt Reilly (kb1vc)
 * @date Jun 29, 2025
 */

/**
 * @page SoDa::PropertyTree PropertyTree A simple property tree widget with
 * documentation. It is a wrapper around jsoncpp.
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
     * @brief Create a property tree from a filename or create
     * a bare property tree.
     *
     * Though this property tree is built upon jsoncpp and uses Json as its
     * representation/container, it does *not* support arrays. Any attempt to set
     * a node value replaces the old value: it does not append. 
     *
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
     * @brief The constructor.
     *
     * Create a property tree from an input Json file.
     *
     * @param filename the name of a json file containing the properties
     */
    PropertyTree(const std::string & filename);



    /**
     * @brief Read a property file
     *
     * Add the stuff in "filename" to the property tree in this
     * object.
     *
     * @param filename the name of the property file to be read
     * @throws SoDa::FileNotFound (a subclass of SoDa::Exception)
     */
    void readFile(const std::string & filename);

    /**
     * @brief Write a property file
     *
     * Write the stuff in "filename" to the property tree in this
     * object. 
     *
     * @param filename the name of the file to be written
     * @throws SoDa::NoFileCreated (a subclass of SoDa::Exception)     
     */
    void writeFile(const std::string & filename);

    /**
     * @brief get a double attribute from the tree
     * @param v the double to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param throw_exception throws an exception if the pathname is not
     * found or if the type of the value cannot be read from the property at that node.
     * @returns true if the value was found and was readable from the property node, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     * @throws SoDa::PropertyTree::BadPropertyType
     */
    bool get(double & v, 
	     const std::string & pathname, 
	     bool throw_exception = false);


    /**
     * @brief get a unsigned long attribute from the tree
     * @param v the unsigned long to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param throw_exception throws an exception if the pathname is not
     * found or if the type of the value cannot be read from the property at that node.
     * @returns true if the value was found and was readable from the property node, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     * @throws SoDa::PropertyTree::BadPropertyType
     */
    bool get(unsigned long & v, 
	     const std::string & pathname, 
	     bool throw_exception = false);


    /**
     * @brief get a long attribute from the tree
     * @param v the long to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param throw_exception throws an exception if the pathname is not
     * found or if the type of the value cannot be read from the property at that node.
     * @returns true if the value was found and was readable from the property node, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     * @throws SoDa::PropertyTree::BadPropertyType
     */
    bool get(long & v, 
	     const std::string & pathname, 
	     bool throw_exception = false);

    
    /**
     * @brief get a string attribute from the tree
     * @param v the string to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param throw_exception throws an exception if the pathname is not
     * found or if the type of the value cannot be read from the property at that node.
     * @returns true if the value was found and was readable from the property node, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     * @throws SoDa::PropertyTree::BadPropertyType
     */
    bool get(std::string & v, 
	     const std::string & pathname, 
	     bool throw_exception = false);

    /**
     * @brief put a unsigned long attribute from the tree
     * @param v the unsigned long to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param create if true, create the path to this attribute if it doesn't exist. (default true)
     * @param throw_exception throws an exception if the pathname is not
     * found and if create is false. (default false)
     * @returns true if the value was found and was writable, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     */
    bool put(unsigned long v, 
	     const std::string & pathname, 
	     bool create = true,
	     bool throw_exception = false);


    /**
     * @brief put a long attribute from the tree
     * @param v the long to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param create if true, create the path to this attribute if it doesn't exist. (default true)
     * @param throw_exception throws an exception if the pathname is not
     * found and if create is false. (default false)
     * @returns true if the value was found and was writable, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     */
    bool put(long v, 
	     const std::string & pathname,
	     bool create = true,	     
	     bool throw_exception = false);


    /**
     * @brief put a double attribute from the tree
     * @param v the double to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param create if true, create the path to this attribute if it doesn't exist. (default true)
     * @param throw_exception throws an exception if the pathname is not
     * found and if create is false. (default false)
     * @returns true if the value was found and was writable, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     */
    bool put(double v, 
	     const std::string & pathname,
	     bool create = true,	     
	     bool throw_exception = false);

    
    /**
     * @brief put a string attribute from the tree
     * @param v the string to be read from the attribute
     * @param pathname the heirarchical path name to the attribute.
     * Levels in the tree are separated with ":" as in "top:next:leaf"
     * @param create if true, create the path to this attribute if it doesn't exist. (default true)
     * @param throw_exception throws an exception if the pathname is not
     * found and if create is false. (default false)
     * @returns true if the value was found and was writable, false otherwise. 
     * @throws SoDa::PropertyTree::PropertyNotFound 
     */
    bool put(std::string v, 
	     const std::string & pathname,
	     bool create = true,	     
	     bool throw_exception = false);

    
    class PropertyNotFound : public SoDa::Exception {
    public:	
      PropertyNotFound(const std::string & path); 
    }; 

    class BadPropertyType : public SoDa::Exception {
    public:	
      BadPropertyType(const std::string & path_name, 
		      const std::string & type_name, 
		      Json::Value * val_string); 
    }; 

    class FileNotFound : public SoDa::Exception {
    public:
      FileNotFound(const std::string & fname);
    }; 

    class FileNotWriteable : public SoDa::Exception {
    public:
      FileNotWriteable(const std::string & fname);
    }; 

    class FileParseError : public SoDa::Exception {
    public:
      FileParseError(const std::string & fname, const std::string & errs);
    }; 
    
  protected:
    Json::Value * getJsonNode(bool & found,
			      const std::string & path,
			      bool create = false,
			      bool throw_exception = false);

    Json::Value root; 
  };
}



