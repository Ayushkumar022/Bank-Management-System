#include "database.h"
#include "sqlite3.h"
#include <iostream>
#include <vector>

// Global database handle, but now it's "private" to this file
static sqlite3* db;

// Callback for getting a list of all accounts
static int select_all_callback(void* data, int argc, char** argv, char** azColName) {
    auto accounts = static_cast<std::vector<Account>*>(data);
    accounts->push_back({std::stoi(argv[0]), argv[1], std::stod(argv[2])});
    return 0;
}

void initialize_database() {
    if (sqlite3_open("bank.db", &db)) {
        std::cerr << "DB Error: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    const char* sql = "CREATE TABLE IF NOT EXISTS ACCOUNTS(ID INTEGER PRIMARY KEY AUTOINCREMENT, NAME TEXT NOT NULL, BALANCE REAL NOT NULL);";
    sqlite3_exec(db, sql, 0, 0, 0);
}

void close_database() {
    sqlite3_close(db);
}

json create_account(const std::string& name) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO ACCOUNTS (NAME,BALANCE) VALUES (?, 0.0);";
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return {{"status", "error"}, {"message", "SQL execution failed."}};
    }
    sqlite3_finalize(stmt);
    
    return {{"status", "success"}, {"id", sqlite3_last_insert_rowid(db)}, {"name", name}};
}

json get_all_accounts() {
    std::vector<Account> accounts_vec;
    sqlite3_exec(db, "SELECT * FROM ACCOUNTS;", select_all_callback, &accounts_vec, 0);
    
    json res = json::array();
    for (const auto& acc : accounts_vec) {
        res.push_back({{"id", acc.id}, {"name", acc.name}, {"balance", acc.balance}});
    }
    return res;
}

json perform_transaction(const std::string& type, int id, double amount) {
    std::string op = (type == "deposit") ? "+" : "-";
    std::string sql = "UPDATE ACCOUNTS SET BALANCE = BALANCE " + op + " ? WHERE ID = ?";
    if (op == "-") {
        sql += " AND BALANCE >= ?";
    }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, id);
    if (op == "-") {
        sqlite3_bind_double(stmt, 3, amount);
    }
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return {{"status", "error"}, {"message", "SQL execution failed."}};
    }
    sqlite3_finalize(stmt);
    
    if (sqlite3_changes(db) == 0) {
        return {{"status", "error"}, {"message", "Tx failed: Account not found or insufficient funds."}};
    }
    return {{"status", "success"}, {"message", type + " successful!"}};
}

json delete_account(int id) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM ACCOUNTS WHERE ID = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return {{"status", "error"}, {"message", "SQL execution failed."}};
    }
    sqlite3_finalize(stmt);

    if (sqlite3_changes(db) == 0) {
        return {{"status", "error"}, {"message", "Account not found."}};
    }
    return {{"status", "success"}, {"message", "Account deleted successfully."}};
}

json perform_transfer(int from_id, int to_id, double amount) {
    // --- Basic validation checks ---
    if (amount <= 0) {
        return {{"status", "error"}, {"message", "Transfer amount must be positive"}};
    }
    if (from_id == to_id) {
        return {{"status", "error"}, {"message", "Cannot transfer to the same account"}};
    }

    char* zErrMsg = 0;
    // --- Begin database transaction ---
    sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, &zErrMsg);

    // --- Withdraw from 'from' account, but only if the balance is sufficient ---
    std::string sql_withdraw = "UPDATE accounts SET balance = balance - ? WHERE id = ? AND balance >= ?;";
    sqlite3_stmt* stmt_withdraw;
    sqlite3_prepare_v2(db, sql_withdraw.c_str(), -1, &stmt_withdraw, 0);
    sqlite3_bind_double(stmt_withdraw, 1, amount);
    sqlite3_bind_int(stmt_withdraw, 2, from_id);
    sqlite3_bind_double(stmt_withdraw, 3, amount);

    int rc_withdraw = sqlite3_step(stmt_withdraw);
    int changes_withdraw = sqlite3_changes(db);
    sqlite3_finalize(stmt_withdraw);

    // Check if the withdrawal actually happened (1 row should be affected)
    if (rc_withdraw != SQLITE_DONE || changes_withdraw == 0) {
        sqlite3_exec(db, "ROLLBACK", 0, 0, &zErrMsg); // Rollback if withdrawal failed
        return {{"status", "error"}, {"message", "Withdrawal failed: Insufficient funds or invalid 'from' account"}};
    }

    // --- Deposit into 'to' account ---
    std::string sql_deposit = "UPDATE accounts SET balance = balance + ? WHERE id = ?;";
    sqlite3_stmt* stmt_deposit;
    sqlite3_prepare_v2(db, sql_deposit.c_str(), -1, &stmt_deposit, 0);
    sqlite3_bind_double(stmt_deposit, 1, amount);
    sqlite3_bind_int(stmt_deposit, 2, to_id);

    int rc_deposit = sqlite3_step(stmt_deposit);
    int changes_deposit = sqlite3_changes(db);
    sqlite3_finalize(stmt_deposit);

    // Check if the deposit actually happened
    if (rc_deposit != SQLITE_DONE || changes_deposit == 0) {
        sqlite3_exec(db, "ROLLBACK", 0, 0, &zErrMsg); // Rollback if deposit failed
        return {{"status", "error"}, {"message", "Deposit failed: Invalid 'to' account"}};
    }

    // --- If both steps succeeded, commit the transaction to make it permanent ---
    sqlite3_exec(db, "COMMIT", 0, 0, &zErrMsg);

    return {{"status", "success"}, {"message", "Transfer successful"}};
}