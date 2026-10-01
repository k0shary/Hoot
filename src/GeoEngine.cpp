#include <cmath>
#include "../core/include/GeoEngine.hpp"

namespace hoot {
  namespace core {
    double GeoEngine::haversineDistance (Location p1, Location p2) {
      double d_lat = DegToRad (p2.lat - p1.lat);
      double d_lon = DegToRad (p2.lon - p1.lon);

      double lat1_rad = DegToRad (p1.lat);
      double lat2_rad = DegToRad (p2.lat);

      //haversine Formula
      //a = hav(theta)
      double a = std::sin(d_lat / 2.0) *
                 std::sin(d_lat / 2.0) +
                 std::cos(lat1_rad) *
                 std::cos(lat2_rad) *
                 std::sin(d_lon / 2.0) *
                 std::sin(d_lon / 2.0);
      
      //c = central angle in radians
      double c = 2 * std::asin(std::sqrt(a));

      return EARTH_RADIUS_KM * c; 
    }
    bool isInsideBoundingBox(Location point, const Polygon& polygon) {
      ///Guard clause incase
      if (polygon.vertices.empty()) return false;

      //Initializing bounding 
      double min_lat = polygon.vertices[0].lat;
      double max_lat = polygon.vertices[0].lat;
      double max_lon = polygon.vertices[0].lon;
      double min_lon = polygon.vertices[0].lon;

      //Scan to calculate min and max coordinates
      for (size_t i = 0; i < polygon.vertices.size(); i++) {
        const Location& v = polygon.vertices[i];

        if (v.lat < min_lat) {
          min_lat = v.lat; 
        };
        
        if (v.lat > max_lat) {
          max_lat = v.lat;
        };
        if (v.lon < min_lon) {
          min_lon = v.lon; 
        };
        if (v.lon > max_lon) {
          max_lon = v.lon; 
        };
      }
      //Returns true if point lies within bounds
      return (point.lat >= min_lat &&
              point.lat <= max_lat &&
              point.lon >= min_lon &&
              point.lon <= max_lon);
    }


  }
}
