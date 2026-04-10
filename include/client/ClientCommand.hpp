#ifndef USER_COMMAND_HPP_
#define USER_COMMAND_HPP_

#include <iostream>
#include <string>
#include <map>
#include <vector>


class UserCommand {
public:
    typedef void (*handler_t)();
    static bool execute(int fd, std::string message);

private:
    static std::map<std::string, handler_t> _commands;
    static void initMap();

    static void handleHelp();
    static void handleLogin();
    static void handleLogout();
    static void handleUsers();
    static void handleUser();
    static void handleSend();
    static void handleMessages();
    static void handleSubscribe();
    static void handleSubscribed();
    static void handleUnsubscribe();
    static void handleUse();
    static void handleCreate();
    static void handleList();
    static void handleInfo();

    static std::string trim(std::string str);
};

#endif /* !CLIENT_COMMAND_HPP_ */
