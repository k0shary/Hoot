#ifndef HOOT_CORE_TYPES_HPP
#define HOOT_CORE_TYPES_HPP
#include <vector>
#include <string>

namespace hoot 
{
  namespace core 
  {
    //2d cooridnates
    struct Location 
    {
      double lat;
      double lon;
    };
    //Polygons for borders
    struct Polygon 
    {
      std::vector<Location> vertices;

    };
    //Policy for boundaries
    enum class PolicyType 
    {
      RADIAL, //harvesine 
      POLYGONAL //ray casting
    };
    //Instance Policies
    struct InstancePolicy 
    {
      std::string id;
      std::string name;
      PolicyType type;
      Location center;      //used for radial
      double radius_km;     //same here
      Polygon boundary_polygon;  ///used for polygons
    };
    //user profile and location model...very rough may be changed in the future 
    struct User 
    {
      std::string id;
      std::string name;
      Location location;
    };
    ///rule evaluation
    struct PolicyDecision
    {
      bool allowed;
      double distance_km;
      std::string rejection_reason;
    };
  }
  
}
#endif // !HOOT_CORE_TYPES_HPP

