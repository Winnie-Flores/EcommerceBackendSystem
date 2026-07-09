#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <mysql.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QString>

// 数据库操作结果行
using Row = std::map<std::string, std::string>;
using ResultSet = std::vector<Row>;

class DBManager {
public:
    static DBManager& instance();

    bool connect(const std::string& host, int port,
                 const std::string& user, const std::string& pass,
                 const std::string& db);

    void disconnect();
    bool isConnected() const;

    // 执行查询
    ResultSet query(const std::string& sql);
    // 执行更新/插入/删除，返回影响行数
    int execute(const std::string& sql);
    // 执行插入，返回自增ID
    long long insert(const std::string& sql);
    // 转义字符串防注入
    std::string escape(const std::string& str);

    // 事务
    bool begin();
    bool commit();
    bool rollback();

private:
    DBManager() = default;
    ~DBManager();
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    MYSQL* conn_ = nullptr;
    bool connected_ = false;
};

// 辅助：将 Row 中的字段转为 QString
inline QString rowStr(const Row& row, const std::string& key, const QString& def = "") {
    auto it = row.find(key);
    return it != row.end() ? QString::fromStdString(it->second) : def;
}

inline int rowInt(const Row& row, const std::string& key, int def = 0) {
    auto it = row.find(key);
    return it != row.end() ? std::stoi(it->second) : def;
}

inline double rowDouble(const Row& row, const std::string& key, double def = 0.0) {
    auto it = row.find(key);
    return it != row.end() ? std::stod(it->second) : def;
}

#endif // DBMANAGER_H
