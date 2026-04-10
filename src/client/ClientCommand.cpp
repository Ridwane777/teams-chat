#include "../../include/client/ClientCommand.hpp"
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <cstring>

std::map<std::string, UserCommand::handler_t> UserCommand::_commands;

void UserCommand::initMap()
{
    _commands["/help"] = &handleHelp;
    _commands["/login"] = &handleLogin;
    _commands["/logout"] = &handleLogout;
    _commands["/users"] = &handleUsers;
    _commands["/user"] = &handleUser;
    _commands["/send"] = &handleSend;
    _commands["/messages"] = &handleMessages;
    _commands["/subscribe"] = &handleSubscribe;
    _commands["/subscribed"] = &handleSubscribed;
    _commands["/unsubscribe"] = &handleUnsubscribe;
    _commands["/use"] = &handleUse;
    _commands["/create"] = &handleCreate;
    _commands["/list"] = &handleList;
    _commands["/info"] = &handleInfo;
}

std::string UserCommand::trim(std::string str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> UserCommand::parseArguments(std::string message)
{
    std::vector<std::string> args;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < message.size(); ++i) {
        if (message[i] == '\"') {
            inQuotes = !inQuotes;
        } else if (message[i] == ' ' && !inQuotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += message[i];
        }
    }
    if (!current.empty())
        args.push_back(current);
    return args;
}

bool UserCommand::execute(int fd, std::string message)
{
    if (_commands.empty())
        initMap();

    message = trim(message);
    if (message.empty())
        return true;

    std::vector<std::string> args = parseArguments(message);
    if (args.empty())
        return true;

    std::string cmdName = args[0];
    args.erase(args.begin());

    if (_commands.find(cmdName) != _commands.end()) {
        _commands[cmdName](fd, args);
        return true;
    } else {
        std::cout << "Unknown command: " << cmdName << std::endl;
        return false;
    }
}

// // Handler implementations
// void UserCommand::handleHelp(int fd, const std::vector<std::string> &args)
// {
//     (void)args;
//     std::string msg = "HELP\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleLogin(int fd, const std::vector<std::string> &args)
// {
//     if (args.size() != 1) {
//         std::cout << "Usage: /login [\"user_name\"]" << std::endl;
//         return;
//     }
//     std::string msg = "LOGIN \"" + args[0] + "\"\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleLogout(int fd, const std::vector<std::string> &args)
// {
//     (void)args;
//     std::string msg = "LOGOUT\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleUsers(int fd, const std::vector<std::string> &args)
// {
//     (void)args;
//     std::string msg = "USERS\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleUser(int fd, const std::vector<std::string> &args)
// {
//     if (args.size() != 1) {
//         std::cout << "Usage: /user [\"user_uuid\"]" << std::endl;
//         return;
//     }
//     std::string msg = "USER \"" + args[0] + "\"\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleSend(int fd, const std::vector<std::string> &args)
// {
//     if (args.size() != 2) {
//         std::cout << "Usage: /send [\"user_uuid\"] [\"message_body\"]" << std::endl;
//         return;
//     }
//     std::string msg = "SEND \"" + args[0] + "\" \"" + args[1] + "\"\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleMessages(int fd, const std::vector<std::string> &args)
// {
//     if (args.size() != 1) {
//         std::cout << "Usage: /messages [\"user_uuid\"]" << std::endl;
//         return;
//     }
//     std::string msg = "MESSAGES \"" + args[0] + "\"\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleSubscribe(int fd, const std::vector<std::string> &args)
// {
//     if (args.size() != 1) {
//         std::cout << "Usage: /subscribe [\"team_uuid\"]" << std::endl;
//         return;
//     }
//     std::string msg = "SUBSCRIBE \"" + args[0] + "\"\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleSubscribed(int fd, const std::vector<std::string> &args)
// {
//     std::string msg = "SUBSCRIBED";
//     if (args.size() == 1)
//         msg += " \"" + args[0] + "\"";
//     msg += "\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleUnsubscribe(int fd, const std::vector<std::string> &args)
// {
//     if (args.size() != 1) {
//         std::cout << "Usage: /unsubscribe [\"team_uuid\"]" << std::endl;
//         return;
//     }
//     std::string msg = "UNSUBSCRIBE \"" + args[0] + "\"\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleUse(int fd, const std::vector<std::string> &args)
// {
//     std::string msg = "USE";
//     for (const auto &arg : args)
//         msg += " \"" + arg + "\"";
//     msg += "\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleCreate(int fd, const std::vector<std::string> &args)
// {
//     std::string msg = "CREATE";
//     for (const auto &arg : args)
//         msg += " \"" + arg + "\"";
//     msg += "\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleList(int fd, const std::vector<std::string> &args)
// {
//     (void)args;
//     std::string msg = "LIST\r\n";
//     write(fd, msg.c_str(), msg.size());
// }

// void UserCommand::handleInfo(int fd, const std::vector<std::string> &args)
// {
//     (void)args;
//     std::string msg = "INFO\r\n";
//     write(fd, msg.c_str(), msg.size());
// }
