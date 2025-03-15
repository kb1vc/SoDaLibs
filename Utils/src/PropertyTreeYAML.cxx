#include "PropertyTreeYAML.hxx"
#include <iostream>
#include <sstream>
#include <yaml-cpp/yaml.h>

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
  PropertyTreeYAML::PropertyTreeYAML() {
    // do nothing
  }
  
  PropertyTreeYAML::PropertyTreeYAML(const std::string & filename) {
    readFile(filename); 
  }

  void PropertyTreeYAML::readFile(const std::string & filename) {
    YAML::Node TopNode; ///< the root node for the current file. 
    
    try {
      TopNode = YAML::LoadFile(filename);
    }
    catch(YAML::BadFile & e) {
      throw PropertyTree::FileNotFound(e.what());
    }
    
    // now recursively create the tree
    fillRecurse(TopNode, root);
  }
  
  void PropertyTreeYAML::fillRecurse(const YAML::Node & node,
				     PropertyTree::PropNode * prop) {
    if(node.IsScalar()) {
      auto pnp = new PropertyTree::PropNode(prop, 
					    node.as<std::string>(), 
					      true);
	
      prop->prop_list.push_back(pnp); 
      return; 
    }
    for(YAML::const_iterator it = node.begin(); 
	it != node.end(); 
	++it) {
    
      if(it->second.IsMap()) {
	// New node. It will have children
	// that are either maps or lists or terminals.
	auto pnp = new PropertyTree::PropNode(prop, "", false);
	prop->dictionary[it->first.as<std::string>()] = pnp;
	fillRecurse(YAML::Node (it->second), pnp);
      }

      if(it->second.IsScalar()) {
	auto pnp = new PropertyTree::PropNode(prop, 
					      it->second.as<std::string>(), 
					      true);
	
	prop->dictionary[it->first.as<std::string>()] = pnp; 
      }
      
      if(it->second.IsSequence()) {
	// we aren't really going to do anything.  Just
	// pass it along, as all prop tree nodes are lists or dictionaries
	// or both
	
	for(auto n : it->second) {
	  fillRecurse(n, prop);
	}
      }
    }
  }

  void PropertyTreeYAML::writeFile(const std::string & filename) {
  }
  
  
  
}
