#ifndef HOOT_CORE_GEO_ENGINE
#define HOOT_CORE_GEO_ENGINE
#include "types.hpp"

namespace hoot {
  namespace core {
    static const double PI = 3.1415926535;
    static const double EARTH_RADIUS_KM = 6378.14;

    class GeoEngine
    {
      public:
        
        ///Degree to radian conversion for GPS
        static inline double DegToRad(double deg)
        {
          return deg * (PI/180);
        };
        ///Haversine formula
        static double haversineDistance (Location p1, Location p2);
        //Ray casting algorithm
        static bool isPointInPolygon(Location point, const Polygon& polygon);
        //fast bounding
        static bool isInsideBoundingBox(Location point, const Polygon& polygon);

    };
  };
};
#endif // !HOOT_CORE_GEO_ENGINE

