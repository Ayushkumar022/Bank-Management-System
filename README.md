# 🏦 C++ Bank Management System

A robust and scalable web-based bank management system developed in C++, leveraging the high-performance **Crow framework** and a reliable **SQLite database**. This project provides a comprehensive set of functionalities for managing bank accounts and transactions through a well-defined **RESTful API**, all deployed efficiently via **Docker containers**.

---

## ✨ Key Features

* **Account Management**:
    * **Create Accounts**: Easily set up new bank accounts with unique identifiers.
    * **View All Accounts**: Retrieve a detailed list of all existing accounts, including their balances.
    * **Delete Accounts**: Securely remove accounts from the system.
* **Transaction Processing**:
    * **Deposits**: Add funds to any specified account.
    * **Withdrawals**: Deduct funds from an account, with checks for sufficient balance.
    * **Fund Transfers**: Facilitate seamless money transfers between two different accounts.
* **RESTful API**: All core banking operations are exposed through a clean, intuitive, and stateless API, making integration with various client applications straightforward.
* **Data Persistence**: Account and transaction data are reliably stored using SQLite, a lightweight embedded database.
* **Containerized Deployment**: The entire application is packaged into a Docker image, ensuring a consistent, isolated, and easily deployable environment.

---

## 🛠️ Technologies & Libraries

This project is built upon a modern C++ stack, utilizing the following key technologies:

* **C++17**: The core programming language, chosen for its performance, efficiency, and robust features.
* **Crow Framework**: A fast and lightweight C++ microframework that handles the web server logic, routing, and HTTP request/response management.
* **SQLite3**: An embedded relational database management system used for local data storage. It's file-based, requiring no separate server.
* **nlohmann/json**: A popular header-only C++ library for parsing, generating, and manipulating JSON data, crucial for API communication.
* **Docker**: Used for containerizing the application, simplifying environment setup, dependencies management, and deployment across different platforms.

---

## 📂 Project Structure

The project is organized as follows:
