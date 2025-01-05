#include "SQLiteWrapper.h"
#include "Responses.h"


SQLiteWrapper::SQLiteWrapper() {
    int rc = sqlite3_open("peer_data.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Create table if it doesn't exist
    std::string createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS Peers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            password TEXT NOT NULL,
            role TEXT NOT NULL,
            room_id INTEGER NOT NULL
        );
    )";
    executeQuery(createTableQuery);


    // Create SDP_Offers table if it doesn't exist
    createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS SDP_Offers (
            username TEXT PRIMARY KEY,
            sdp_offer BLOB
        );
    )";
    executeQuery(createTableQuery);

    createTableQuery = R"(
    CREATE TABLE IF NOT EXISTS SDP_Offers (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT NOT NULL,
        sdp_offer BLOB,
        sdp_response BLOB
        );
    )";
	executeQuery(createTableQuery);
}

SQLiteWrapper::~SQLiteWrapper() {
    sqlite3_close(db);
}

/*
bool SQLiteWrapper::authenticateUser(AuthenticationRequest& details) {
    // Check if user already exists in the database
    std::string checkUserQuery = "SELECT * FROM Peers WHERE username = '" + details.username + "' AND password = '" + details.password + "'";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, checkUserQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            // User already exists
            sqlite3_finalize(stmt);
            return true;
        }
    }

    sqlite3_finalize(stmt);

    // If user does not exist, insert new user
    std::string insertUserQuery = "INSERT INTO Peers (username, password, role, ip) VALUES ('" + details.username + "', '" + details.password + "', '" + details.role + "', '" + details.ip + "')";
    return executeQuery(insertUserQuery);
}*/

int SQLiteWrapper::userExists(AuthenticationRequest& details)
{
    std::string checkUserQuery = "SELECT password FROM Peers WHERE username = '" + details.username + "'";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, checkUserQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            // User exists, retrieve the stored password
            std::string storedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            sqlite3_finalize(stmt);

            // Check if the provided password matches the stored password
            if (storedPassword == details.password) {
                return USER_ALREADY_EXISTS;  // User exists and password is correct
            }
            else {
                return PASSWORD_INCORRECT;  // Password does not match
            }
        }
    }
    sqlite3_finalize(stmt);
    return USER_NOT_FOUND;  // User does not exist
}

bool SQLiteWrapper::addUser(AuthenticationRequest& details)
{
    std::string insertUserQuery = "INSERT INTO Peers (username, password, role, room_id) VALUES ('" + details.username + "', '" + details.password + "', '" + details.role + "', 0)"; 
    return executeQuery(insertUserQuery);  // Assuming executeQuery returns true if successful
}

/*
std::string SQLiteWrapper::getMatchingPeer(const std::string& username,const std::string & password, const std::string& role) {

    // right now cause im running exerythong on local 127.0.01 the ip will be = but in original needs to be != cause differnt ip's
    // for different computer
    std::string findIPQuery = "SELECT role FROM Peers WHERE username = '" + username + "' AND password == '" + password +  "' AND role != '" + role + "'";

    sqlite3_stmt* stmt;
    std::string matchedIP = "";

    if (sqlite3_prepare_v2(db, findIPQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            matchedIP = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
    }

    sqlite3_finalize(stmt);
    return matchedIP;
}*/

void SQLiteWrapper::saveSDPOffer(const std::string& username, const std::vector<char>& sdpOffer)
{
    std::string insertQuery = "INSERT OR REPLACE INTO SDP_Offers (username, sdp_offer) VALUES (?, ?)";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 2, sdpOffer.data(), sdpOffer.size(), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Error inserting SDP offer: " << sqlite3_errmsg(db) << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

std::vector<char> SQLiteWrapper::getSDPResponse(const std::string& username)
{
    std::string getQuery = "SELECT sdp_response FROM SDP_Offers WHERE username = ?";

    sqlite3_stmt* stmt;
    std::vector<char> sdpResponse;

    if (sqlite3_prepare_v2(db, getQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const void* blobData = sqlite3_column_blob(stmt, 0);
            int blobSize = sqlite3_column_bytes(stmt, 0);

            if (blobData != nullptr && blobSize > 0) {
                sdpResponse.assign((char*)blobData, (char*)blobData + blobSize);
            }
            else {
                std::cerr << "BLOB data is empty or null for username: " << username << std::endl;
            }
        }
        else {
            std::cerr << "No SDP response found for " << username << std::endl;
        }
    }
    else {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return sdpResponse;
}

void SQLiteWrapper::saveSDPResponse(const std::string& username, const std::vector<char>& sdpResponse)
{
    std::string insertQuery = "UPDATE SDP_Offers SET sdp_response = ? WHERE username = ?";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_blob(stmt, 1, sdpResponse.data(), sdpResponse.size(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to update SDP response: " << sqlite3_errmsg(db) << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

std::vector<char> SQLiteWrapper::getSDPOffer(const std::string& username)
{
    std::string getQuery = "SELECT sdp_offer FROM SDP_Offers WHERE username = ?";

    sqlite3_stmt* stmt;
    std::vector<char> sdpOffer;

    if (sqlite3_prepare_v2(db, getQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const void* blobData = sqlite3_column_blob(stmt, 0);
            int blobSize = sqlite3_column_bytes(stmt, 0);

            if (blobData != nullptr && blobSize > 0) {
                sdpOffer.assign((char*)blobData, (char*)blobData + blobSize);
            }
            else {
                std::cerr << "BLOB data is empty or null for username: " << username << std::endl;
            }
        }
        else {
            std::cerr << "No SDP offer found for " << username << std::endl;
        }
    }
    else {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return sdpOffer;
}

bool SQLiteWrapper::setRoomID(std::string username, std::string password, int roomID)
{
    // First, verify if the user exists and the password is correct
    std::string checkUserQuery = "SELECT COUNT(*) FROM Peers WHERE username = ? AND password = ?";
    sqlite3_stmt* stmt;

    // Prepare the statement
    if (sqlite3_prepare_v2(db, checkUserQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

        // Execute the query
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int userExists = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);

            if (userExists > 0) {
                // User exists, now update the room ID
                std::string updateRoomIDQuery = "UPDATE Peers SET room_id = ? WHERE username = ?";

                if (sqlite3_prepare_v2(db, updateRoomIDQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_int(stmt, 1, roomID);
                    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);

                    // Execute the update
                    if (sqlite3_step(stmt) == SQLITE_DONE) {
                        sqlite3_finalize(stmt);
                        return true; // Room ID updated successfully
                    }
                }
            }
        }
    }

    sqlite3_finalize(stmt);
    return false; // Failed to update room ID
}

bool SQLiteWrapper::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
