#include "../include/Evaluator.hpp"

namespace hoot {
  namespace core {
    PolicyDecision Evaluator::isLocationAllowed (const Location& userloc, const InstancePolicy& policy) {
      PolicyDecision decision;
      decision.allowed = false;
      decision.distance_km = 0.0;
      decision.rejection_reason = "";

      ///Radial check
      if (policy.type == PolicyType::RADIAL) {
        double dist = GeoEngine::haversineDistance(userloc, policy.center);
        decision.distance_km = dist; 

        if (dist <= policy.radius_km){
          decision.allowed = true;
        } else {
          decision.rejection_reason = "You are outside the radial distance";
        }
        return decision;
      }


      ///polygonal check
      if (policy.type == PolicyType::POLYGONAL) {
        if (GeoEngine::isPointInPolygon(userloc, policy.boundary_polygon)) {
          decision.allowed = true;
        } else {
          decision.rejection_reason = "You are outside the designated boundary";
        }
        return decision;

      }
      decision.rejection_reason = "Unknown policy";
      return decision;
    }
    
  }
  
}
