#include "dbmanager.h"
#include <QDebug>
#include <cstring>

DBManager& DBManager::instance() {
    static DBManager inst;
    return inst;
}

DBManager::~DBManager() {
    disconnect();
}

bool DBManager::connect(const std::string& host, int port,
                         const std::string& user, const std::string& pass,
                         const std::string& db) {
    conn_ = mysql_init(nullptr);
    if (!conn_) return false;

    mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(), pass.c_str(),
                            db.c_str(), port, nullptr, 0)) {
        qWarning() << "MySQL connect failed:" << mysql_error(conn_);
        mysql_close(conn_);
        conn_ = nullptr;
        return false;
    }
    connected_ = true;
    return true;
}

void DBManager::disconnect() {
    if (conn_) {
        mysql_close(conn_);
        conn_ = nullptr;
    }
    connected_ = false;
}

bool DBManager::isConnected() const {
    return connected_ && conn_;
}

ResultSet DBManager::query(const std::string& sql) {
    ResultSet result;
    if (!connected_) return result;

    if (mysql_query(conn_, sql.c_str()) != 0) {
        qWarning() << "Query error:" << mysql_error(conn_);
        return result;
    }

    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return result;

    int numFields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Row r;
        unsigned long* lengths = mysql_fetch_lengths(res);
        for (int i = 0; i < numFields; ++i) {
            r[fields[i].name] = row[i] ? std::string(row[i], lengths[i]) : "";
        }
        result.push_back(r);
    }

    mysql_free_result(res);
    return result;
}

int DBManager::execute(const std::string& sql) {
    if (!connected_) return -1;

    if (mysql_query(conn_, sql.c_str()) != 0) {
        qWarning() << "Execute error:" << mysql_error(conn_);
        return -1;
    }
    return static_cast<int>(mysql_affected_rows(conn_));
}

long long DBManager::insert(const std::string& sql) {
    if (!connected_) return -1;

    if (mysql_query(conn_, sql.c_str()) != 0) {
        qWarning() << "Insert error:" << mysql_error(conn_);
        return -1;
    }
    return mysql_insert_id(conn_);
}

std::string DBManager::escape(const std::string& str) {
    if (!conn_ || str.empty()) return str;

    std::vector<char> buf(str.size() * 2 + 1);
    mysql_real_escape_string(conn_, buf.data(), str.c_str(),
                             static_cast<unsigned long>(str.size()));
    return std::string(buf.data());
}

bool DBManager::begin() {
    return execute("START TRANSACTION") >= 0;
}

bool DBManager::commit() {
    return execute("COMMIT") >= 0;
}

bool DBManager::rollback() {
    return execute("ROLLBACK") >= 0;
}
