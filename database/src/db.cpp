#include "../include/db.hpp"
namespace hoot {
  namespace db {

    sqlite3* init(const char* path) {
      sqlite3* db = nullptr;
      ///attempts to open or create a database file
      if (sqlite3_open(path, &db) != SQLITE_OK) return nullptr;


      ///define core schemas/tables
      const char* schema =
        "CREATE TABLE IF NOT EXISTS policies ("
        "   id TEXT PRIMARY KEY, uri TEXT, type INT, lat REAL, lon REAL, rad REAL);"
        "CREATE TABLE IF NOT EXISTS posts ("
        "   id TEXT PRIMARY KEY, content TEXT, policy_id TEXT,"
        "   FOREIGN KEY(policy_id) REFERENCES policies(id));";

      //run schema init script
      sqlite3_exec(db, schema, nullptr, nullptr, nullptr);
      return db;
    }

    
    bool save_policy(sqlite3* db, const core::InstancePolicy& p) {
      const char* sql = "INSERT OR REPLACE INTO policies VALUES (?, ?, ?, ?, ?, ?);";
      sqlite3_stmt* stmt;

      //prpevents sql injections
      if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;


      ///binding c++ structs to sql parameters
      sqlite3_bind_text(stmt, 1, p.id.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, p.places_pub_uri.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_int(stmt, 3, static_cast<int>(p.type));
      sqlite3_bind_double(stmt, 4, p.center.lat);
      sqlite3_bind_double(stmt, 5, p.center.lon);
      sqlite3_bind_double(stmt, 6, p.radius_km);

      bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
      sqlite3_finalize(stmt);
      return ok;
    }
    std::vector<core::InstancePolicy> load_policies(sqlite3* db) {
      std::vector<core::InstancePolicy> list;
      const char* sql = "SELECT id, uri, type, lat, lon, rad FROM policies;";
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
          list.push_back(p);
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
