#include "PropertyNode.hxx"
/*
  BSD 2-Clause License

  Copyright (c) 2021,2025 Matt Reilly - kb1vc
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

  PropertyNode::PropertyNode(PropertyNodePtr parent) : parent(parent) {
    value_is_valid = false; 
    me = std::weak_ptr<PropertyNode>(this);
  }

  PropertyNodePtr PropertyNode::makeNode(const std::string & node_name) {
    auto nnp = PropertyNode::make(me);
    return appendNode(node_name, nnp);
  }

  PropertyNodePtr PropertyNode::appendNode(const std::string & node_name, 
					   PropertyNodePtr subroot) {
    
    if(children.count(node_name)) {
      throw DuplicatePropertyName(me, node_name); 
    }
    else {
      children[node_name] = subroot; 
    }
    return subroot; 
  }

  std::string PropertyNode::getPathName(std::string so_far) const {
    if(parent == nullptr) {
      // we've reached the root.
      return ":" + so_far; 
    }

    // first get my parent's key for me.
    auto key = parent->getKeyFromNode(me);
    
    if(key == "") {
      throw CorruptedTree(so_far, parent); 
    }
    
    so_far = key + ":" + so_far; 

    return parent->getPathName(so_far); 
  }
  
  std::string PropertyNode::getKeyFromNode(const PropertyNodePtr pn) const {
    for(auto cp : children) {
      if(cp.second == pn) {
	return cp.first; 
      }
    }
    
    return ""; 
  }
  
  PropertyNodePtr PropertyNode::getNode(const std::string & pathname) const {
    if(pathname == "") {
      return me; 
    }
    
    // peel off the first name in the path and give me the rest
    PropertyTree::ripName(pathname, first, rest);

    
    if(children.count(first)) {
      return children[first]->getNode(rest);
    }
  
    // if we get here, the pathname was not found. 
    return nullptr;
  }
      
  std::vector<std::string> PropertyNode::getKeys() const {
    std::vector<std::string> ret;
    for(auto cp : children) {
      ret.push_back(cp.first);
    }
  }

  std::ostream & PropertyNode::dump(std::ostream & os, std::string indent) {
    if(value_is_valid) {
      os << indent << value_string << "\n";
    }
    else {
      for(auto cp : children) {
	os << indent << cp.first << "\n";
	dump(os, indent + "  ");
      }
    }
    return os; 
  }
}
