#pragma once

#include <sqlite3.h>
#include <vector>
#include "../../core/include/types.hpp"

namespace hoot {
  namespace network {

    class server {
      public:
        int port_;
        sqlite3* db_;
        std::vector<core::InstancePolicy> policies_;
        void start_server(int port, sqlite3* db);
        server(int port, sqlite3* db);
        void start();
    };
  }
}
