#pragma once 
#include <string.h>
#include "types.hpp"

namespace hoot {
  namespace core {
    //Pure parsing
    bool parse_nominatim(const std::string& body, InstancePolicy& out);
    ///parse+network
    bool fetch_place(const std::string& query, InstancePolicy& out); 
  }
}
