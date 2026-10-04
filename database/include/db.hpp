#pragma once 
#include <sqlite3.h>
#include <vector>
#include <string>
#include "../../core/include/types.hpp"

namespace hoot {
  namespace db {

    sqlite3* init(const char* path = "hoot.db"); 
    bool save_policy(sqlite3* db, const core::InstancePolicy& p);
    std::vector<core::InstancePolicy> load_policies(sqlite3* db);
      void close(sqlite3* db);
  }
}
