#include "../core/include/PlacesFetcher.hpp"
#include <curl/curl.h>
#include <cstdlib>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace hoot{
  namespace core{
    ///take thes chunk of raw bytes and append them to c++ std::stringa
    static size_t write_cb(char* data, size_t size, size_t nmemb, void* userp){
      size_t bytes = size * nmemb;
      static_cast<std::string*>(userp)->append(data, bytes);
      return bytes;
    }

    bool parse_nominatim(const std::string& body, InstancePolicy& out){
      json j = json::parse(body, nullptr, false);
      if (j.is_discarded () || !j.is_array() || j.empty())
        return false;
/*
      json item = j[0];
      if (!item.contains("geojson") || !item.contains("lat"))
        return false;

      std::string lat_str = item["lat"].get<std::string>();
      std::string lon_str = item["lon"].get<std::string>();

        */
      //search up to 5 result for polygon or MultiPolygon
      json item;
      bool found_valid = false;

      size_t num_items = j.size();
      for (size_t i = 0; i < num_items; i++){
        json entry = j[i];
        if (!entry.contains("geojson") || !entry.contains("lat")) continue;

        std::string gtype = entry["geojson"]["type"].get<std::string>();
        if (gtype == "Polygon" || gtype == "MultiPolygon"){
          item = entry;
          found_valid = true;
          break;
        }
      }

      // fallback: build bounding box polygon if no Polygon/MultiPolygon geometry is in top results
      if (!found_valid) {
        json top = j[0];
        if (top.contains("boundingbox") && top["boundingbox"].is_array() && top["boundingbox"].size() == 4) {
          out.center.lat = std::atof(top["lat"].get<std::string>().c_str());
          out.center.lon = std::atof(top["lon"].get<std::string>().c_str());

          double min_lat = std::atof(top["boundingbox"][0].get<std::string>().c_str());
          double max_lat = std::atof(top["boundingbox"][1].get<std::string>().c_str());
          double min_lon = std::atof(top["boundingbox"][2].get<std::string>().c_str());
          double max_lon = std::atof(top["boundingbox"][3].get<std::string>().c_str());

          out.boundary_polygon.vertices.clear();
          out.boundary_polygon.vertices.push_back({min_lat, min_lon});
          out.boundary_polygon.vertices.push_back({max_lat, min_lon});
          out.boundary_polygon.vertices.push_back({max_lat, max_lon});
          out.boundary_polygon.vertices.push_back({min_lat, max_lon});

          out.type = PolicyType::POLYGONAL;
          return true;
        }
        return false;
      }

      //doubel precison float
      //item["lat"].get<std::string>();
      out.center.lat = std::atof(item["lat"].get<std::string>().c_str());
      out.center.lon = std::atof(item["lon"].get<std::string>().c_str());

      json geo = item["geojson"];
      std::string type = geo["type"].get<std::string>();

      json ring;
      if (type == "Polygon") {
        ring = geo["coordinates"][0];
      } else if
        (type == "MultiPolygon"){
          ring = geo["coordinates"][0][0];
        }else {
          return false;
        }
        out.boundary_polygon.vertices.clear();

        size_t vertex_count = ring.size();
        for (size_t i = 0; i < vertex_count; i++) {
          if (!ring[i].is_array() || ring[i].size() < 2) continue;

          Location loc;
          loc.lon = ring[i][0].get<double>();
          loc.lat = ring[i][1].get<double>();
          out.boundary_polygon.vertices.push_back(loc);
        }
        out.type = PolicyType::POLYGONAL;
        return out.boundary_polygon.vertices.size() >= 3;  
      }


    bool fetch_place(const std::string& query, InstancePolicy& out){
      CURL* curl = curl_easy_init();
      if (!curl)
        return false;

      //simple url encoding
      char* escaped = curl_easy_escape(curl, query.c_str(), 0);
      std::string url = "https://nominatim.openstreetmap.org/search?format=json&limit=5&polygon_geojson=1&q=";
      url += escaped;
      curl_free(escaped);
      std::string response_body;

      ///http requests
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT,"HootProject/1.0");
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

      CURLcode res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);

      if (res != CURLE_OK) return false;

      out.id = query;
      out.name = query;

      // Debug print to see what Nominatim actually returned
       if (!parse_nominatim(response_body, out)) {
         std::cerr << "Nominatim response:\n" << response_body.substr(0, 500) << "\n...\n";
         return false;
       }
      return true;
    }
  }
}
