#include "db.hpp"
#include "server.hpp"
#include <iostream>

int main() {
    std::cout << "Starting Hoot ActivityPub Server..." <<  std::endl;

    sqlite3* db = hoot::db::init("hoot.db");
    if (!db) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    hoot::network::server srv(8080, db);
    srv.start();

    hoot::db::close(db);
    return 0;
}
