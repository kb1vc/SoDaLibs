#pragma once


namespace SoDa::Property {
  class Path {
  public:
    Path(const std::string & pathstr);
    Path(const std::vector<std::string> & pathvec);
    Path(const std::list<std::string> & pathlis);

    /**
     * @brief make a path from a list of names.
     * @param element vector, these will be catenated e.g. ["flintstone", "fred", "age"]
     * @return a real pathname e.g. "flintstone:fred:age"
     */
    std::string makePath(const std::vector<std::string> & elementlist);

    /**
     * @brief make a path from a list of names.
     * @param element list, these will be catenated e.g. ["flintstone", "fred", "age"]
     * @return a real pathname e.g. "flintstone:fred:age"
     */
    std::string makePath(const std::list<std::string> & elementlist);

    std::vector<std::string> splitPath(const std::string & pathstr);


    /**
     * @brief Add an element to the end of a path
     * @param path the original path, f'rinstance "flintstone:fred"
     * @param name the thing to be appended, f'rinstance "age"
     * @return the catenated pathname, f'rinstance "flintstone:fred:age"
     */
    std::string addToPath(const std::string & path, const std::string & name);
    
  }; 
}
