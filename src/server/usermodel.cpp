#include "usermodel.h"
#include "database.h"
#include <iostream>

using namespace std;

// User表的增加方法
bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into user(name,password,state) values('%s', '%s', '%s')",
             user.get_Name().c_str(), user.get_Password().c_str(), user.get_State().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id 当做用户的账号给用户返回
            user.set_Id(mysql_insert_id(mysql.get_Connection()));
            return true;
        }
    }
    return false;
}

// 根据用户id查询用户信息
User UserModel::query(int id)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select * from user where id = %d", id);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                // 生成一个User对象，填入信息
                User user;
                user.set_Id(atoi(row[0]));
                user.set_Name(row[1]);
                user.set_Password(row[2]);
                user.set_State(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::update_State(User user)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "update user set state = '%s' where id =%d", user.get_State().c_str(), user.get_Id());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 重置用户的状态信息
void UserModel::reset_State()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
