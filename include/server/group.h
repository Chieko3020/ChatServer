#ifndef GROUP_H
#define GROUP_H

#include "groupuser.h"
#include <string>
#include <vector>
using namespace std;

class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }
    void set_Id(int id) 
    { 
        this->id = id; 
    }
    void set_Name(string name) 
    { 
        this->name = name;
    }
    void set_Desc(string desc) 
    { 
        this->desc = desc; 
    }

    int get_Id() 
    { 
        return this->id; 
    }
    string get_Name() 
    { 
        return this->name; 
    }
    string get_Desc() 
    { 
        return this->desc; 
    }
    vector<GroupUser> &get_Users() 
    { 
        return this->users; 
    }

private:
    int id;//群号
    string name;//群名
    string desc;//群功能描述
    vector<GroupUser> users; // 存储组的所有成员
};

#endif
