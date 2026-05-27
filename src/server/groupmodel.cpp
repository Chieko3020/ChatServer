#include "groupmodel.h"
#include "database.h"

// 创建群组
bool GroupModel::create_Group(Group &group)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into allgroup(groupname,groupdesc) values('%s','%s')", group.get_Name().c_str(), group.get_Desc().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 创建成功 设置群组id
            group.set_Id(mysql_insert_id(mysql.get_Connection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::add_Group(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into groupuser values(%d,%d,'%s')", groupid, userid, role.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在群组信息
vector<Group> GroupModel::query_Groups(int userid)
{

    // 先根据userid在groupuser表中查询出该用户所属的群组信息
    // 再根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息

    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select a.id,a.groupname,a.groupdesc from allgroup a inner join groupuser b on a.id=b.groupid where b.userid=%d", userid);

    vector<Group> groupVec;

    MySQL mysql;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                //atoi(const char* str) str to int
                group.set_Id(atoi(row[0]));
                group.set_Name(row[1]);
                group.set_Desc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    // 查询群组的用户信息
    for (Group &group : groupVec)
    {
        snprintf(sql, sizeof(sql),  "select a.id,a.name,a.state,b.grouprole from user a inner join groupuser b on a.id=b.userid where b.groupid=%d", group.get_Id());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.set_Id(atoi(row[0]));
                user.set_Name(row[1]);
                user.set_State(row[2]);
                user.set_Role(row[3]);
                group.get_Users().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

// 根据指定的groupid查询群组用户id列表(除userid自己)
vector<int> GroupModel::query_GroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid=%d and userid !=%d", groupid, userid);

    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                //atoi(const char* str) str to int
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}
