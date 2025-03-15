#include "PropertyTree.hxx"
#include <iostream>
#include <sstream>
#include "Utils.hxx"

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


namespace SoDa {
  PropertyTree::PropertyTree() {
    root = new PropNode(nullptr, "root", false);
  }

  std::string PropertyTree::PropNode::getPathName() {
    std::string ret;
    if(parent != nullptr) {
      ret = parent->getPathName() + ":" + val_string;
    }
    
    return ret; 
  }
  
				  
  PropertyTree::PropNode::PropNode(PropNode * parent, 
				   const std::string & val_string,
				   bool is_terminal) : 
    parent(parent), val_string(val_string), is_terminal(is_terminal)
  {
    // nothing much to do. 
  }
  
  
  
  PropertyTree::PropNode * 
  PropertyTree::PropNode::getProp(const std::string & pathname, 
				  bool throw_exception) {
    // first split the first chunk of the pathname -- up to the :
    std::list<std::string> pathlist = SoDa::split(pathname, ":"); 
    return getPropRecursive(pathlist, pathname, throw_exception); 
  }

  PropertyTree::PropNode * 
  PropertyTree::PropNode::getPropRecursive(std::list<std::string> & pathlist, 
					   const std::string & orig_pathname, 
					   bool throw_exception) {
    if(pathlist.size() == 0) {
      return this; 
    }
    else if(dictionary.find(pathlist.front()) != dictionary.end()) {
	pathlist.pop_front();
	return getPropRecursive(pathlist, orig_pathname, throw_exception);
    }
    else {
      if(throw_exception) {
	throw PropertyNotFound(orig_pathname); 
      }
    }

    return nullptr; 
  }
  
  std::list<PropertyTree::PropNode *> PropertyTree::PropNode::getList() {
    return prop_list; 
  }
  
  std::list<std::string> PropertyTree::PropNode::getKeys() {
    std::list<std::string> keys; 
    for(auto a : dictionary) {
      keys.push_back(a.first); 
    }
    return keys; 
  }


  std::ostream & PropertyTree::dump(std::ostream & os) {
    return root->dump(os, "");
  }

  std::ostream & PropertyTree::PropNode::dump(std::ostream & os, std::string indent) {
    
    if(is_terminal) {
      os << indent << val_string << "\n";
      return os;
    }

    if(!prop_list.empty()) {
      os << "<<<PROPLIST>>>\n";      
      for(auto v : prop_list) {
	if(v->is_terminal) {
	  os << indent << v->val_string << "\n";
	}
	else {
	  v->dump(os, indent + "  ");
	}
      }
    }

    if(!dictionary.empty()) {
      os << "<<<DICTIONARY>>>\n";
      for(auto v : dictionary) {
	os << indent << v.first << ":  "; 
	if(v.second->is_terminal) {
	  os << v.second->val_string << "\n";
	}
	else {
	  os << "\n";
	  v.second->dump(os, indent + "  ");
	}
      }
    }
    
    return os; 
  }
  
  PropertyTree::FileNotFound::FileNotFound(const std::string & str) :
    Exception(SoDa::Format("PropertyTree::PropNode::FileNotFound \"%0\"")
	      .addS(str).str())
  {
  }
  PropertyTree::PropNode::PropertyNotFound::PropertyNotFound(const std::string & str) :
    Exception(SoDa::Format("PropertyTree::PropNode::PropertyNotFound \"%0\"")
	      .addS(str)
	      .str())
  {
  }

  PropertyTree::PropNode::BadPropertyType::BadPropertyType(const std::string & path_name, 
							   const std::string & type_name, 
							   const std::string & val_string) :
    Exception(SoDa::Format("PropertyTree::PropNode::BadPropertyType at node name \"%0\" with value string \"%1\" which cannot be converted to type \"2\"")
	      .addS(path_name)
	      .addS(val_string)
	      .addS(type_name)
	      .str()) {
  }
  
}
