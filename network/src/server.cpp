#include "../include/server.hpp"
#include "../include/httplib.h"
#include "../../database/include/db.hpp"
#include "../../core/include/Evaluator.hpp"
#include <iostream>
#include <vector>

static hoot::network::server* g_server_instance = nullptr;

void handle_inbox(const httplib::Request& req, httplib::Response& res){
  std::cout << "Recieved incoming activity from activitypub on inbox" << std::endl;

  if (!g_server_instance || g_server_instance->policies_.empty()) {
    res.status = 200;
    res.set_content("{\"status\": \"accepted\", \"reason\": \"no policies enforced\"}", "application/json");
    return;
  }

  hoot::core::Location incoming_loc {55.39594, 10.38831};
  bool globally_allowed = true;
  std::string rejection_reason = "";

  for (const auto& policy : g_server_instance->policies_) {
    auto decision = hoot::core::Evaluator::isLocationAllowed(incoming_loc, policy);
    if (!decision.allowed) {
      globally_allowed = false;
      rejection_reason = decision.rejection_reason;
      break;
    }
  }
  if (globally_allowed) {
        res.status = 200;
        res.set_content("{\"status\": \"Accepted\", \"message\": \"Location verified successfully.\"}", "application/json");
        std::cout << "[+] Post Accepted by Location Policy.\n";
    } else {
        res.status = 403;
        res.set_content("{\"status\": \"Rejected\", \"reason\": \"" + rejection_reason + "\"}", "application/json");
        std::cout << "Post Rejected: " << rejection_reason << "\n";
    }
}

namespace hoot {
  namespace network {

    server::server(int port, sqlite3* db) : port_(port), db_(db) {
        policies_ = db::load_policies(db_);
        std::cout << "Loaded " << policies_.size() << " geographic policy/policies from DB.\n";
        g_server_instance = this;
    }

    void server::start() {
        httplib::Server svr;
        svr.Post("/inbox", handle_inbox);

        std::cout << "Hoot server starting on port " << port_ << "...\n";
        svr.listen("0.0.0.0", port_);
      }
    }
  }

