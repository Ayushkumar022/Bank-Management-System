#pragma once // Prevents this file from being included multiple times

#include <string>
#include <vector>
#include "json.hpp" // Use nlohmann::json

// for convenience
using json = nlohmann::json;

// This struct is now declared here so other files can use it
struct Account {
    int id;
    std::string name;
    double balance;
};

// Public "menu" of our database functions
void initialize_database();
void close_database();
json create_account(const std::string& name);
json get_all_accounts();
json perform_transaction(const std::string& type, int id, double amount);
json delete_account(int id);
json perform_transfer(int from_id, int to_id, double amount);

// We will leave the 'transfer' function for you to refactor as an exercise