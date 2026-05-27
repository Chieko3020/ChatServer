#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include"group.h"
#include<string>
#include<vector>
using namespace std;

class GroupModel
{
public:
    //创建群组
    bool create_Group(Group &group);

    //加入群组
    void add_Group(int userid,int groupid,string role);

    //查询用户所在群组信息
    vector<Group> query_Groups(int userid);

    //根据指定的groupid查询群组用户id列表 除userid自己
    vector<int> query_GroupUsers(int userid,int groupid);
};

#endif
