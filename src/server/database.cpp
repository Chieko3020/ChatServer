#include "database.h"
#include <muduo/base/Logging.h>

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "suzune";
static string password = "123456";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    sql_conn = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (sql_conn != nullptr)
        mysql_close(sql_conn);
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *sql_pointer = mysql_real_connect(sql_conn, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (sql_pointer != nullptr)
    {
        LOG_INFO << "connect mysql successed!";
        mysql_query(sql_conn, "set names gbk");
    }
    else
    {
        LOG_INFO << "connect mysql failed!";
    }
    return sql_pointer;
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(sql_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "更新失败!";
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(sql_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(sql_conn);
}
// 返回MySQL连接
MYSQL* MySQL::get_Connection()
{
    return sql_conn;
}