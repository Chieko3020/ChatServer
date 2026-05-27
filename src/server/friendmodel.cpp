#include "friendmodel.h"
#include "database.h"

// 添加好友关系
void FriendModel::insert(int userId, int friendId)
{

    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into friend values(%d, %d)", userId, friendId);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 返回用户好友列表
std::vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};

    // 联合查询 User和Friend两表联合查询，给它userid，返回 friendid 对应的这几个人的id，name，state
    snprintf(sql, sizeof(sql), "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userId);

    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.set_Id(atoi(row[0]));
                user.set_Name(row[1]);
                user.set_State(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
