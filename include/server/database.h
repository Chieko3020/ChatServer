#ifndef DATABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <string>

using namespace std;

// 数据库操作类
// 管理数据库的连接 增删改查接口
// MySQL 类将这些接口封装，向上层 model 层提供各种服务
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    // 返回MySQL连接
    MYSQL* get_Connection();

private:
    MYSQL *sql_conn;
};

#endif
