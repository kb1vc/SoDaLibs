#include "PropertyTree.hxx"
#include <iostream>
#include <fstream>
#include <memory>
#include <json/json.h>

#include "Utils.hxx"
#include "Format.hxx"

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


namespace SoDa {
  PropertyTree::PropertyTree() {
    root_p = new Json::Value; 
    // nothing to do. 
  }

  PropertyTree::PropertyTree(const std::string & fname) {
    root_p = new Json::Value; 
    readFile(fname);
  }

  Json::Value * PropertyTree::getJsonNode(bool & found, 
					  const std::string & path, 
					  bool create, 
					  bool throw_exception) {
    // first split the path
    auto pathv = SoDa::split(path, ":", true);

    auto current_node_p = root_p; 

    // stride down the pathlist
    for(auto pe : pathv) {
      // don't attempt to index into a terminal value 
      bool is_indexable = !(current_node_p->isNumeric() || current_node_p->isString());


      if(current_node_p == nullptr) {
	found = false;
	return root_p;
      }

      // is it a member?            
      if(is_indexable && current_node_p->isMember(pe)) {
	current_node_p = &((*current_node_p)[pe]);
      }
      else if(is_indexable && create) {
	(*current_node_p)[pe] = Json::Value();
	current_node_p = &((*current_node_p)[pe]);
      }
      else if(throw_exception) {
	throw PropertyNotFound(path);
      }
      else {
	found = false;
	return root_p; // gotta return something
      }
    }

    found = true; 
    return current_node_p; 
  }

  bool PropertyTree::get(unsigned long & v, 
			 const std::string & pathname, 
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, false, throw_exception);
    if(!found) return false;
    
    if(node->isUInt64()) {
      v = node->asUInt64();
      return true; 
    }
    else if(throw_exception) {
      throw BadPropertyType(pathname, "Unsigned Long", node);
    }
    
    return false;
  }

  bool PropertyTree::get(double & v, 
			 const std::string & pathname, 
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, false, throw_exception);

    if(!found) return false;
    
    if(node->isDouble()) {
      v = node->asDouble();
      return true; 
    }
    else if(throw_exception) {
      throw BadPropertyType(pathname, "Double", node);
    }
    
    return false;
  }
  

  bool PropertyTree::get(long & v, 
			 const std::string & pathname, 
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, false, throw_exception);
    if(!found) return false;
    
    if(node->isInt64()) {
      v = node->asInt64();
      return true; 
    }
    else if(throw_exception) {
      throw BadPropertyType(pathname, "Long", node);
    }
    
    return false;
  }

    
  bool PropertyTree::get(std::string & v, 
			 const std::string & pathname, 
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, false, throw_exception);
    if(!found) return false;

    if(node->isString()) {
      v = node->asString();
      return true; 
    }
    else if(throw_exception) {
      throw BadPropertyType(pathname, "String", node);
    }
    
    return false;
  }

  bool PropertyTree::put(unsigned long v, 
			 const std::string & pathname,
			 bool create,
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, create, throw_exception);
    if(found) {
      *node = v; 
      return true; 
    }
    else {
      return false; 
    }
  }


  bool PropertyTree::put(long  v, 
			 const std::string & pathname, 
			 bool create,
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, create, throw_exception);
    if(found) {
      *node = v; 
      return true; 
    }
    else {
      return false; 
    }
  }


  bool PropertyTree::put(double  v, 
			 const std::string & pathname, 
			 bool create,
			 bool throw_exception) {
    bool found; 
    auto node = getJsonNode(found, pathname, create, throw_exception);
    
    if(found) {
      *node = v;
      return true; 
    }
    else {
      return false; 
    }
  }
  
    
  bool PropertyTree::put(std::string  v, 
			 const std::string & pathname, 
			 bool create,
			 bool throw_exception) {

    bool found; 
    auto node = getJsonNode(found, pathname, create, throw_exception);
    if(found) {
      *node = v;
      return true; 
    }
    else {
      return false; 
    }
  }
  

  std::string convertNodeType(Json::Value * node) {
    if(node->isArray()) {
      return "[Json array]";
    }
    else return node->asString(); 
  }
  
  PropertyTree::FileNotFound::FileNotFound(const std::string & str) :
    Exception(SoDa::Format("PropertyTree::FileNotFound Could not open \"%0\" for input")
	      .addS(str).str())
  {
  }

  PropertyTree::FileNotWriteable::FileNotWriteable(const std::string & str) :
    Exception(SoDa::Format("PropertyTree::FileNotWritable Could not open \"%0\" for output")
	      .addS(str).str())
  {
  }

  PropertyTree::FileParseError::FileParseError(const std::string & str, const std::string & errs) :
    Exception(SoDa::Format("PropertyTree::FileParseError Could not parse \"%0\" as valid Json [%1]")
	      .addS(str)
	      .addS(errs)
	      .str())
  {
  }
  
  PropertyTree::PropertyNotFound::PropertyNotFound(const std::string & str) :
    Exception(SoDa::Format("PropertyTree::PropertyNotFound \"%0\"")
	      .addS(str)
	      .str())
  {
  }

  PropertyTree::BadPropertyType::BadPropertyType(const std::string & path_name, 
							   const std::string & type_name, 
							   Json::Value * node) :
    Exception(SoDa::Format("PropertyTree::BadPropertyType at node name \"%0\" with value string \"%1\" which cannot be converted to type \"2\"")
	      .addS(path_name)
	      .addS(convertNodeType(node))
	      .addS(type_name)
	      .str()) {

  }

  void PropertyTree::readFile(const std::string & filename) {
    std::ifstream ifs(filename);
    if(ifs.is_open()) {
      Json::CharReaderBuilder builder;
      std::string errs;
      if(!parseFromStream(builder, ifs, root_p, &errs)) {
	ifs.close();
	// we got some kind of error	
	throw FileParseError(filename, errs);
      }
    }
    else {
      throw FileNotFound(filename); 
    }
    ifs.close();
  }

  void PropertyTree::writeFile(const std::string & filename) {
    std::ofstream ofs(filename);
    if(ofs.is_open()) {
      Json::StreamWriterBuilder builder;
      // make it this way so that it gets closed when we're done
      const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
      writer->write(*root_p, &ofs);
    }
    else {
      throw FileNotWriteable(filename); 
    }
  }
  
}
