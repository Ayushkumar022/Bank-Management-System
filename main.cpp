#include "crow_all.h"
#include "database.h" // Our new database header
#include <fstream>
#include <string>

int main()
{
    initialize_database();
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() -> crow::response {
        std::ifstream file("index.html");
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto res = crow::response(buffer.str());
            res.set_header("Content-Type", "text/html");
            return res;
        }
        return crow::response(500, "Error: index.html not found.");
    });

    CROW_ROUTE(app, "/api/create")([](const crow::request& req) {
        auto name = req.url_params.get("name");
        if (!name) {
            return crow::response(400, json{{"status", "error"}, {"message", "Missing name"}}.dump());
        }
        json result = create_account(name);
        return crow::response(result.dump());
    });
    
    CROW_ROUTE(app, "/api/accounts")([]() {
        json result = get_all_accounts();
        return crow::response(result.dump());
    });

    CROW_ROUTE(app, "/api/transaction")([](const crow::request& req){
        auto id_str = req.url_params.get("id");
        auto amount_str = req.url_params.get("amount");
        auto type = req.url_params.get("type");
        if (!id_str || !amount_str || !type) {
            return crow::response(400, json{{"status", "error"}, {"message", "Missing parameters"}}.dump());
        }
        json result = perform_transaction(type, std::stoi(id_str), std::stod(amount_str));
        return crow::response(result.dump());
    });
    
    CROW_ROUTE(app, "/api/delete")([](const crow::request& req){
        auto id_str = req.url_params.get("id");
        if (!id_str) {
            return crow::response(400, json{{"status", "error"}, {"message", "Missing ID parameter"}}.dump());
        }
        json result = delete_account(std::stoi(id_str));
        return crow::response(result.dump());
    });

    CROW_ROUTE(app, "/api/transfer")([](const crow::request& req){
        auto from_id_str = req.url_params.get("from");
        auto to_id_str = req.url_params.get("to");
        auto amount_str = req.url_params.get("amount");

        if (!from_id_str || !to_id_str || !amount_str) {
            return crow::response(400, json{{"status", "error"}, {"message", "Missing 'from', 'to', or 'amount' parameters"}}.dump());
        }

        // We will need to create this function in your database.cpp file next
        json result = perform_transfer(std::stoi(from_id_str), std::stoi(to_id_str), std::stod(amount_str));

        return crow::response(result.dump());
    });
    // Print the helpful message FIRST
    std::cout << "\n--> Server is ready! Access it here: http://localhost:18080\n" << std::endl;

    // NOW, start the server. The program will wait here.
    app.port(18080).multithreaded().run();

    // This code will only run AFTER you stop the server with Ctrl+C
    close_database(); 

    return 0;
}