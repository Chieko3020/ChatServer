#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.h"

// User表的数据操作类
// model层通过调用MYSQL类封装的方法来执行各种业务
class UserModel
{
public:
    // User表的插入方法
    bool insert(User &user);

    // 根据用户id查询用户信息
    User query(int id);

    // 更新用户的状态信息
    bool update_State(User user);

    // 重置用户的状态信息
    void reset_State();
};

#endif // USERMODEL_H
