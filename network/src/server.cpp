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

// std::string read_ui(const std::string& path){
//     std::ifstream f(path);
//     if (!f.is_open()) return "";
//     std::ostringstream buf;
//     buf << f.rdbuf();
//     return buf.str();
// }


//replace html element placeholders in index.html 
//to data fetched from our Database
    void replace_html(std::string& str, const std::string& old, const std::string& replacement){
        if(old.empty())
            return;

        size_t pos = 0;
        while ((pos = str.find(old, pos)) != std::string::npos){
            str.replace(pos, old.length(), replacement);
            pos += replacement.length();
        }
    }


//reads the content of index.html and inserts the profile data fetched from the database
//and if non-existent it gives it predefined default profile data 
std::string create_profile_page(sqlite3* database, const std::string& username){
    std::ifstream file("./ui/Profile/index.html");
    if (!file.is_open())
        return "Not found";

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html = buffer.str();

    hoot::db::ProfileData p;
    if (!hoot::db::load_profile(database, username, p)){
        p = {username, "Anon", "", "smile.png", "", ""};
    }

    std::string posts_html;
    auto posts = hoot::db::load_posts(database, username);
    for (const auto& post : posts) {
        posts_html += "<div class=\"hoots-post\"><div class=\"Hoot-content\">"
                       "<div class=\"hoot-avatar\"><img src=\"" + p.avatar_url + "\" alt=\"Avatar\"></div>"
                       "<div class=\"hoot-display-name\"><p><strong>" + p.display_name + "</strong></p></div>"
                       "<div class=\"hoot-username\"><p>@" + p.username + "@hoot.sdu.dk</p></div>"
                      "<div class=\"hoot-text\"><p>" + post.content + "</p></div>"
                      "</div></div>";
    }

    if (posts_html.empty()) {
        posts_html = "<div class=\"Hoots\"><div class=\"Hoot-content\"><div class=\"hoot-text\"><p>Nothing here yet</p></div></div></div>";
    }

        replace_html(html, "_username_", p.username);
        replace_html(html, "_display_name_", p.display_name);
        replace_html(html, "_bio_", p.bio);
        replace_html(html, "_avatar_url_", p.avatar_url);
        replace_html(html, "_cover_url_", p.cover_url);
        replace_html(html, "_background_url_", p.background_url);
        replace_html(html, "_posts_feed_", posts_html);

        return  html;
    }




    // size_t pos;
    //
    //     pos = html.find("_username_");
    //     if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_username_").length(), p.username);
    //
    //     }
    //
    //     pos = html.find("_display_name_");
    //     if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_display_name_").length(), p.display_name);
    //
    //     }
    //
    //     pos = html.find("_bio_");
    //     if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_bio_").length(), p.bio);
    //
    //     }
    //
    //     pos = html.find("_avatar_url_");
    //     if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_avatar_url_").length(), p.avatar_url);
    //
    //     }
    //
    //     pos = html.find("_cover_url_");
    //     if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_cover_url_").length(), p.cover_url);
    //
    //     }
    //
    //     pos = html.find("_background_url_");
    //     if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_background_url_").length(), p.background_url);
    //
    //     }
    //
    //     pos = html.find("_posts_feed_");
    //      if (pos != std::string::npos) {
    //         html.replace(pos, std::string("_posts_feed_").length(), posts_html);
        // }
        // pos = html.find("_custom_css_");
        // if (pos != std::string::npos) {
        //     html.replace(pos, std::string("_custom_css_").length(), p.custom_css);
        //
        // }

//checks incoming activity, if the request is missing any elements it wont pass
//if the instance doesn't inforce a geolocation policy everything will pass, 
//otherwise, if there is a policy in the database, the engine and Evaluator
//checks whether the request  matches the policy and sends a message if accepted or not
//
void handle_inbox(const httplib::Request& req, httplib::Response& res){
  std::cout << "Recieved incoming activity from activitypub on inbox" << std::endl;

  json j = json::parse(req.body, nullptr, false);

  if (j.is_discarded() || !j.contains("location") || !j.contains("id") || !j.contains("content") || !j.contains("username")) {
      res.status = 400;
      res.set_content("{\"status\": \"error\", \"reason\": \"invalid json or missing fields\"}", "application/json");
      return;

  }

  std::string post_id = j["id"].get<std::string>();
  std::string content= j["content"].get<std::string>();
  std::string username= j["username"].get<std::string>();


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
     bool saved =  hoot::db::save_post(g_server_instance->db_,
             post_id
             , content
             , matching_policy_id
             , username);

     if (!saved) {
         std::cerr << "Failed to save post\n";
         res.status = 500;
         res.set_content("Database save failed", "text/plain");
         return;
     }

        res.status = 200;
        res.set_content("{\"status\": \"Accepted\", \"message\": \"Location verified successfully.}\n", "application/json");
        std::cout << "Post Accepted by Location Policy.\n";
    } else {
        res.status = 403;
        res.set_content("{\"status\": \"Rejected\", \"reason\": \"" + rejection_reason + "\"}\n", "application/json");
        std::cout << "Post Rejected: " << rejection_reason << "\n";
    }
}


//handles incoming requests to check a specific user's profile
void handle_profile(const httplib::Request& req, httplib::Response& res){
    std::string uname = req.path_params.at("username");
    
    if (!g_server_instance) {
        res.status = 500;
        res.set_content("server error", "text/plain");
        return ;
    }

    std::string html = create_profile_page(g_server_instance->db_, uname);
    res.set_content(html, "text/html");
}

namespace hoot {
  namespace network {

    //load policies over the network
    server::server(int port, sqlite3* db) : port_(port), db_(db) {
        policies_ = db::load_policies(db_);
        std::cout << "Loaded " << policies_.size() << " geographic policy/policies from DB.\n";
        g_server_instance = this;
    }

//starts the server and handles redirections and mounting
void server::start() {
        httplib::Server svr;

        svr.Post("/inbox", handle_inbox);

        svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
                res.set_redirect("/Sign-in/index.html");
                });

        svr.Get("/Profile", [](const httplib::Request&, httplib::Response& res) {
                res.set_redirect("/Profile/index.html");
                });

        svr.Get("/profile/:username", handle_profile);

        if (!svr.set_mount_point("/", "./ui")) {
            std::cerr << "Mount point failed: ./ui not found (wrong working directory?)\n";
        }

        std::cout << "Hoot server starting on port " << port_ << "...\n";
        svr.listen("0.0.0.0", port_);
        }
    
    }

}

