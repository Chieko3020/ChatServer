#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void set_Id(int id)
    {
        this->id = id;
    }
    void set_Name(string name)
    {
        this->name = name;
    }
    void set_Password(string pwd)
    {
        this->password = pwd;
    }
    void set_State(string state)
    {
        this->state = state;
    }

    int get_Id()
    {
        return this->id;
    }
    string get_Name()
    {
        return this->name;
    }
    string get_Password()
    {
        return this->password;
    }
    string get_State()
    {
        return this->state;
    }

private:
    int id;
    string name;
    string password;
    string state;
};

#endif
