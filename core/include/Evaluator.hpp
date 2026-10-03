#pragma once
#ifndef HOOT_CORE_EVALUATOR
#define HOOT_CORE_EVALUATOR

#include "types.hpp"
#include "GeoEngine.hpp"

namespace hoot {
  namespace core {
    class Evaluator {
      public:
        static PolicyDecision isLocationAllowed (const Location& userloc, const InstancePolicy& policy);
    };   
  }  
}
#endif
