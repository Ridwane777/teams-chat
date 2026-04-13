#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <iostream>
#include <string>
#include <map>
#include "../ServerState.hpp"
#include "../protocol/Protocol.hpp"

using Function = std::function<void(int, Frame&)>;

class CommandHandler {
public:
    void handle_command(int client_fd, const std::string &command);
    void execute_command(int client_fd, Frame &frame);
    private:
    ServerState _server_state;
    Protocol _protocol;
    std::map<std::string, std::string> commands;
};

#endif