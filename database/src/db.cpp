#include "../include/db.hpp"
#include "nlohmann/json.hpp"

namespace hoot {
  namespace db {

    using json = nlohmann::json;

    sqlite3* init(const char* path) {
      sqlite3* db = nullptr;
      ///attempts to open or create a database file
      if (sqlite3_open(path, &db) != SQLITE_OK) return nullptr;

      sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);


      ///define core schemas/tables
      const char* schema =
        "CREATE TABLE IF NOT EXISTS policies ("
        "   id TEXT PRIMARY KEY, uri TEXT, type INT, lat REAL, lon REAL, rad REAL, vertices TEXT);"
        "CREATE TABLE IF NOT EXISTS posts ("
        "   id TEXT PRIMARY KEY, content TEXT, policy_id TEXT,"
        "   FOREIGN KEY(policy_id) REFERENCES policies(id));";

      //run schema init script
      sqlite3_exec(db, schema, nullptr, nullptr, nullptr);
      return db;
    }

    
    bool save_policy(sqlite3* db, const core::InstancePolicy& p) {
      const char* sql = "INSERT OR REPLACE INTO policies VALUES (?, ?, ?, ?, ?, ?, ?);";
      sqlite3_stmt* stmt;

      //prpevents sql injections
      if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

      //serialize boundary vertices to json array string
      json j_vert = json::array();
      size_t v_count = p.boundary_polygon.vertices.size();
      for (size_t i = 0; i < v_count; i++) {
        json pt = json::array({p.boundary_polygon.vertices[i].lat, p.boundary_polygon.vertices[i].lon});
        j_vert.push_back(pt);
      }
      std::string verts_str = j_vert.dump();


      ///binding c++ structs to sql parameters
      sqlite3_bind_text(stmt, 1, p.id.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, p.places_pub_uri.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_int(stmt, 3, static_cast<int>(p.type));
      sqlite3_bind_double(stmt, 4, p.center.lat);
      sqlite3_bind_double(stmt, 5, p.center.lon);
      sqlite3_bind_double(stmt, 6, p.radius_km);
      sqlite3_bind_text(stmt, 7, verts_str.c_str(), -1, SQLITE_TRANSIENT);

      bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
      sqlite3_finalize(stmt);
      return ok;
    }


    std::vector<core::InstancePolicy> load_policies(sqlite3* db) {
      std::vector<core::InstancePolicy> list;
      const char* sql = "SELECT id, uri, type, lat, lon, rad, vertices FROM policies;";
      sqlite3_stmt* stmt;



      if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        //step through each row in the query
        while (sqlite3_step(stmt) == SQLITE_ROW) {
          core::InstancePolicy p;
          p.id = (const char*)sqlite3_column_text(stmt, 0);
          p.places_pub_uri = (const char*)sqlite3_column_text(stmt, 1);
          p.type = static_cast<core::PolicyType>(sqlite3_column_int(stmt, 2));
          p.center.lat = sqlite3_column_double(stmt, 3);
          p.center.lon = sqlite3_column_double(stmt, 4);
          p.radius_km = sqlite3_column_double(stmt, 5);


          ///deserialize vertices from json text column
          const unsigned char* verts_text = sqlite3_column_text(stmt, 6);
          if (verts_text) {
            std::string verts_json_str(reinterpret_cast<const char*>(verts_text));
            json j_vert = json::parse(verts_json_str, nullptr, false);
            if (!j_vert.is_discarded() && j_vert.is_array()) {
              p.boundary_polygon.vertices.clear();
              size_t v_size = j_vert.size();
              for (size_t i = 0; i < v_size; i++){
                if (j_vert[i].is_array() && j_vert[i].size() >= 2) {
                  core::Location loc;
                  loc.lat = j_vert[i][0].get<double>();
                  loc.lon = j_vert[i][1].get<double>();
                  p.boundary_polygon.vertices.push_back(loc);   
                }
              } 
            } 
          }
          list.push_back(p);
        }
      }
      sqlite3_finalize(stmt);
      return list;
    }



    bool save_post(sqlite3* db, const std::string& post_id, const std::string& content, const std::string& policy_id) {
      const char* sql = "INSERT OR REPLACE INTO posts VALUES (?, ?, ?);"; 
      sqlite3_stmt* stmt;

      //prpevents sql injections
      if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;


      ///binding c++ structs to sql parameters
      sqlite3_bind_text(stmt, 1, post_id.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, policy_id.c_str(), -1, SQLITE_STATIC);

      bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
      sqlite3_finalize(stmt);
      return ok;
    }


    std::vector<core::PostRecord> load_posts(sqlite3* db) {
      std::vector<core::PostRecord> list;
      const char* sql = "SELECT id, content, policy_id FROM posts;";
      sqlite3_stmt* stmt;

      if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        //step through each row in the query
        while (sqlite3_step(stmt) == SQLITE_ROW) {
          core::PostRecord post;
          post.id = (const char*)sqlite3_column_text(stmt, 0);
          post.content= (const char*)sqlite3_column_text(stmt, 1);
          post.policy_id= (const char*)sqlite3_column_text(stmt, 2);
          list.push_back(post);
        }
      }

      ///freeing after loop completes      
      sqlite3_finalize(stmt);
      return list;
    }
    void close(sqlite3* db) {
      if (db) sqlite3_close(db);
    }
  }
}
