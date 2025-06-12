#ifndef GROUPUSER_H
#define GROUPUSER_H

#include"user.h"

//群组用户
class GroupUser: public User
{
public:
    void set_Role(string role)
    {
        this->role=role;
    }
    string get_Role()
    {
        return this->role;
    }
private:
    string role;//群内身份 （creator normal）
};

#endif
