#include <cmath>
#include "GeoEngine.hpp"

namespace hoot {
  namespace core {
    double GeoEngine::haversineDistance (Location p1, Location p2) {
      double d_lat = DegToRad (lat.p2 -lat.p1);
      double d_lon = DegToRad (lon.p2 -lon.p1);

    }
  }
}
