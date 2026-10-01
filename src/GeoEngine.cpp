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
  }
}
