#ifndef USER_COMMAND_HPP_
#define USER_COMMAND_HPP_

#include <iostream>
#include <string>
#include <map>
#include <vector>


class UserCommand {
public:
    typedef void (*handler_t)(int fd, const std::vector<std::string> &args);
    static bool execute(int fd, std::string message);

private:
    static std::map<std::string, handler_t> _commands;
    static void initMap();

    static void handleHelp(int fd, const std::vector<std::string> &args);
    static void handleLogin(int fd, const std::vector<std::string> &args);
    static void handleLogout(int fd, const std::vector<std::string> &args);
    static void handleUsers(int fd, const std::vector<std::string> &args);
    static void handleUser(int fd, const std::vector<std::string> &args);
    static void handleSend(int fd, const std::vector<std::string> &args);
    static void handleMessages(int fd, const std::vector<std::string> &args);
    static void handleSubscribe(int fd, const std::vector<std::string> &args);
    static void handleSubscribed(int fd, const std::vector<std::string> &args);
    static void handleUnsubscribe(int fd, const std::vector<std::string> &args);
    static void handleUse(int fd, const std::vector<std::string> &args);
    static void handleCreate(int fd, const std::vector<std::string> &args);
    static void handleList(int fd, const std::vector<std::string> &args);
    static void handleInfo(int fd, const std::vector<std::string> &args);

    static std::vector<std::string> parseArguments(std::string message);
    static std::string trim(std::string str);
};

#endif /* !CLIENT_COMMAND_HPP_ */
