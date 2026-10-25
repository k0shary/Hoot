#include "../include/server.hpp"
#include "../include/httplib.h"
#include "../../database/include/db.hpp"
#include "../../core/include/Evaluator.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using json = nlohmann::json;
static hoot::network::server* g_server_instance = nullptr;

std::string read_ui(const std::string& path){
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::ostringstream buf;
    buf << f.rdbuf();
    return buf.str();
}


void handle_inbox(const httplib::Request& req, httplib::Response& res){
  std::cout << "Recieved incoming activity from activitypub on inbox" << std::endl;

  json j = json::parse(req.body, nullptr, false);

  if (j.is_discarded() || !j.contains("location") || !j.contains("id") || !j.contains("content")) {
      res.status = 400;
      res.set_content("{\"status\": \"error\", \"reason\": \"invalid json or missing fields\"}", "application/json");
      return;

  }

  std::string post_id = j["id"].get<std::string>();
  std::string content= j["content"].get<std::string>();


  hoot::core::Location incoming_loc {j["location"]["lat"].get<double>(),
                                    j["location"]["lon"].get<double>()};

  if (!g_server_instance || g_server_instance->policies_.empty()) {
    res.status = 200;
    res.set_content("{\"status\": \"accepted\", \"reason\": \"no policies enforced\"}\n", "application/json");
    return;
  }

  bool globally_allowed = true;
  std::string rejection_reason = "";
  std::string matching_policy_id= "";

  for (const auto& policy : g_server_instance->policies_) {
    auto decision = hoot::core::Evaluator::isLocationAllowed(incoming_loc, policy);
    if (!decision.allowed) {
      globally_allowed = false;
      rejection_reason = decision.rejection_reason;
      break;
    }
    matching_policy_id = policy.id;
  }
  if (globally_allowed) {
      hoot::db::save_post(g_server_instance->db_, post_id, content, matching_policy_id);

        res.status = 200;
        res.set_content("{\"status\": \"Accepted\", \"message\": \"Location verified successfully.}\n", "application/json");
        std::cout << "Post Accepted by Location Policy.\n";
    } else {
        res.status = 403;
        res.set_content("{\"status\": \"Rejected\", \"reason\": \"" + rejection_reason + "\"}\n", "application/json");
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

        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
                res.set_redirect("/Sign-in/index.html");
                });

        svr.Get("/profile", [](const httplib::Request&, httplib::Response& res) {
                res.set_redirect("/Profile/index.html");
                });

        if (!svr.set_mount_point("/", "./UI")) {
            std::cerr << "Mount point failed: ./UI not found (wrong working directory?)\n";
        }

        std::cout << "Hoot server starting on port " << port_ << "...\n";
        svr.listen("0.0.0.0", port_);
        }
    
    }

}

