#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <unistd.h>
#include <iostream>
#include <string>
#include <map>
#include "../ServerState.hpp"
#include "../../../include/server/protocol/Protocol.hpp"
extern "C" {
#include "../../../libs/myteams/logging_server.h"
}

using Function = std::function<void(int, Frame &)>;

class CommandHandler
{
private:
    ServerState _server_state;
    Protocol _protocol;
    Frame _current_frame;
    int client_fd;
    std::map<std::string, Function> commands;

public:

    CommandHandler();
    ~CommandHandler();

    void handle_command(int client_fd, const std::string &command);
    void execute_command(int client_fd, Frame &frame);

    void handle_login(int client_fd, Frame &frame);
    void handle_logout(int client_fd, Frame &frame);
    void handle_users_list(int client_fd, Frame &frame);
    void handle_user_get(int client_fd, Frame &frame);
    void handle_msg_send(int client_fd, Frame &frame);
    void handle_msg_list(int client_fd, Frame &frame);
    void handle_team_create(int client_fd, Frame &frame);
    void handle_team_sub(int client_fd, Frame &frame);
    void handle_team_unsub(int client_fd, Frame &frame);
    void handle_team_subbed(int client_fd, Frame &frame);
    void handle_chan_create(int client_fd, Frame &frame);
    void handle_thread_create(int client_fd, Frame &frame);
    void handle_comment_create(int client_fd, Frame &frame);
    void handle_list(int client_fd, Frame &frame);
    void handle_info(int client_fd, Frame &frame);
    void handle_use(int client_fd, Frame &frame);

};

#endif