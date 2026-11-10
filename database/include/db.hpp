#pragma once 
#include <sqlite3.h>
#include <vector>
#include <string>
#include "../../core/include/types.hpp"

namespace hoot {
  namespace db {
      struct ProfileData {
          std::string username;
          std::string display_name;
          std::string avatar_url;
          std::string cover_url;
          std::string background_url;
          std::string bio;
          // std::string custom_css;

      };

    sqlite3* init(const char* path = "hoot.db"); 
    bool save_policy(sqlite3* db, const core::InstancePolicy& p);
    std::vector<core::InstancePolicy> load_policies(sqlite3* db);

    bool save_post(sqlite3* db, const std::string& post_id, const std::string& content, const std::string& policy_id, const std::string& username);
    std::vector<core::PostRecord> load_posts(sqlite3* db, const std::string& username);

    bool save_profile(sqlite3* db, const ProfileData& p);
    bool load_profile(sqlite3* db, const std::string username, ProfileData& p);
      void close(sqlite3* db);
  }
}
