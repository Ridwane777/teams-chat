#include "../../include/client/ClientCommand.hpp"
#include "../../include/client/ResponseHandler.hpp"
#include <unistd.h>
#include <cstring>

std::map<std::string, UserCommand::handler_t> UserCommand::_commands;
std::string UserCommand::_teamUuid = "";
std::string UserCommand::_channelUuid = "";
std::string UserCommand::_threadUuid = "";

#include "../../include/client/client.hpp"

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

void UserCommand::handleHelp(int fd, const std::vector<std::string> &args)
{
    (void)fd;
    (void)args;
    std::cout << "Usage:" << std::endl;
    std::cout << "  /help : show this help" << std::endl;
    std::cout << "  /login [\"user_name\"] : set the user_name used by client" << std::endl;
    std::cout << "  /logout : disconnects the client from the server" << std::endl;
    std::cout << "  /users : list all users that exist on the server" << std::endl;
    std::cout << "  /user [\"user_uuid\"] : get information about a user" << std::endl;
    std::cout << "  /send [\"user_uuid\"] [\"message_body\"] : send a message to a user" << std::endl;
    std::cout << "  /messages [\"user_uuid\"] : list all messages with a user" << std::endl;
    std::cout << "  /subscribe [\"team_uuid\"] : subscribe to a team" << std::endl;
    std::cout << "  /subscribed [\"team_uuid\"] : list subscribed teams or users in a team" << std::endl;
    std::cout << "  /unsubscribe [\"team_uuid\"] : unsubscribe from a team" << std::endl;
    std::cout << "  /use [\"team_uuid\"] [\"channel_uuid\"] [\"thread_uuid\"] : set context" << std::endl;
    std::cout << "  /create : create a resource (team, channel, thread, comment)" << std::endl;
    std::cout << "  /list : list resources in the current context" << std::endl;
    std::cout << "  /info : get information about the current resource" << std::endl;
}

void UserCommand::handleLogin(int fd, const std::vector<std::string> &args)
{
    if (args.size() != 1) {
        std::cout << "Usage: /login [\"user_name\"]" << std::endl;
        return;
    }
    ResponseHandler::setPendingCommand("/login");
    std::string msg = "LOG name=" + Client::url_encode(args[0]) + "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleLogout(int fd, const std::vector<std::string> &args)
{
    (void)args;
    ResponseHandler::setPendingCommand("/logout");
    std::string msg = "OUT\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleUsers(int fd, const std::vector<std::string> &args)
{
    (void)args;
    ResponseHandler::setPendingCommand("/users");
    std::string msg = "ULS\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleUser(int fd, const std::vector<std::string> &args)
{
    if (args.size() != 1) {
        std::cout << "Usage: /user [\"user_uuid\"]" << std::endl;
        return;
    }
    ResponseHandler::setPendingCommand("/user");
    std::string msg = "UGT uuid=" + Client::url_encode(args[0]) + "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleSend(int fd, const std::vector<std::string> &args)
{
    if (args.size() != 2) {
        std::cout << "Usage: /send [\"user_uuid\"] [\"message_body\"]" << std::endl;
        return;
    }
    ResponseHandler::setPendingCommand("/send");
    std::string msg = "SND to=" + Client::url_encode(args[0]) + " body=" + Client::url_encode(args[1]) + "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleMessages(int fd, const std::vector<std::string> &args)
{
    if (args.size() != 1) {
        std::cout << "Usage: /messages [\"user_uuid\"]" << std::endl;
        return;
    }
    ResponseHandler::setPendingCommand("/messages");
    std::string msg = "MLS with=" + Client::url_encode(args[0]) + "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleSubscribe(int fd, const std::vector<std::string> &args)
{
    if (args.size() != 1) {
        std::cout << "Usage: /subscribe [\"team_uuid\"]" << std::endl;
        return;
    }
    ResponseHandler::setPendingCommand("/subscribe");
    std::string msg = "TSB team=" + Client::url_encode(args[0]) + "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleSubscribed(int fd, const std::vector<std::string> &args)
{
    std::string msg = "TSD";
    if (args.size() == 1) {
        ResponseHandler::setPendingCommand("/subscribed_users");
        msg += " team=" + Client::url_encode(args[0]);
    } else {
        ResponseHandler::setPendingCommand("/subscribed_teams");
    }
    msg += "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleUnsubscribe(int fd, const std::vector<std::string> &args)
{
    if (args.size() != 1) {
        std::cout << "Usage: /unsubscribe [\"team_uuid\"]" << std::endl;
        return;
    }
    ResponseHandler::setPendingCommand("/unsubscribe");
    std::string msg = "TUS team=" + Client::url_encode(args[0]) + "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleUse(int fd, const std::vector<std::string> &args)
{
    ResponseHandler::setPendingCommand("/use");
    std::string msg = "USE";
    if (args.size() >= 1) {
        _teamUuid = args[0];
        msg += " team=" + Client::url_encode(_teamUuid);
    } else {
        _teamUuid.clear();
        _channelUuid.clear();
        _threadUuid.clear();
    }
    if (args.size() >= 2) {
        _channelUuid = args[1];
        msg += " channel=" + Client::url_encode(_channelUuid);
    } else {
        _channelUuid.clear();
        _threadUuid.clear();
    }
    if (args.size() >= 3) {
        _threadUuid = args[2];
        msg += " thread=" + Client::url_encode(_threadUuid);
    } else {
        _threadUuid.clear();
    }
    msg += "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleCreate(int fd, const std::vector<std::string> &args)
{
    std::string msg;
    if (_teamUuid.empty()) {
        if (args.size() != 2) {
            std::cout << "Usage: /create \"team_name\" \"team_desc\"" << std::endl;
            return;
        }
        ResponseHandler::setPendingCommand("/create_team");
        msg = "TCR name=" + Client::url_encode(args[0]) + " description=" + Client::url_encode(args[1]);
    } else if (_channelUuid.empty()) {
        if (args.size() != 2) {
            std::cout << "Usage: /create \"channel_name\" \"channel_desc\"" << std::endl;
            return;
        }
        ResponseHandler::setPendingCommand("/create_channel");
        msg = "CCR name=" + Client::url_encode(args[0]) + " description=" + Client::url_encode(args[1]);
    } else if (_threadUuid.empty()) {
        if (args.size() != 2) {
            std::cout << "Usage: /create \"thread_title\" \"thread_body\"" << std::endl;
            return;
        }
        ResponseHandler::setPendingCommand("/create_thread");
        msg = "HCR title=" + Client::url_encode(args[0]) + " body=" + Client::url_encode(args[1]);
    } else {
        if (args.size() != 1) {
            std::cout << "Usage: /create \"comment_body\"" << std::endl;
            return;
        }
        ResponseHandler::setPendingCommand("/create_reply");
        msg = "OCR body=" + Client::url_encode(args[0]);
    }
    msg += "\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleList(int fd, const std::vector<std::string> &args)
{
    (void)args;
    if (_teamUuid.empty())
        ResponseHandler::setPendingCommand("/list_teams");
    else if (_channelUuid.empty())
        ResponseHandler::setPendingCommand("/list_channels");
    else if (_threadUuid.empty())
        ResponseHandler::setPendingCommand("/list_threads");
    else
        ResponseHandler::setPendingCommand("/list_replies");
    std::string msg = "LST\n";
    write(fd, msg.c_str(), msg.size());
}

void UserCommand::handleInfo(int fd, const std::vector<std::string> &args)
{
    (void)args;
    if (_teamUuid.empty())
        ResponseHandler::setPendingCommand("/info_user");
    else if (_channelUuid.empty())
        ResponseHandler::setPendingCommand("/info_team");
    else if (_threadUuid.empty())
        ResponseHandler::setPendingCommand("/info_channel");
    else
        ResponseHandler::setPendingCommand("/info_thread");
    std::string msg = "INF\n";
    write(fd, msg.c_str(), msg.size());
}
