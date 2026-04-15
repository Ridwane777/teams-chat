#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <unistd.h>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <functional>
#include "../ServerState.hpp"
#include "../../../include/server/protocol/Protocol.hpp"
extern "C" {
#include "../../../libs/myteams/logging_server.h"
}
#include "../../Utils.hpp"

using Function = std::function<void(int, Frame &)>;

struct ClientContext {
    std::string team;
    std::string channel;
    std::string thread;
};

class CommandHandler
{
private:
    ServerState _server_state;
    Protocol _protocol;
    std::map<std::string, Function> commands;
    std::map<int, ClientContext> _contexts;
    std::set<int> _connected_clients;

    void send_frame(int fd, const std::string &frame);
    void broadcast_frame(const std::string &frame);
    void broadcast_team_event(const uuid_t &team_uuid, const std::string &frame);
    User *require_logged_user(int client_fd);
    bool parse_uuid_field(const Frame &frame, const std::string &field_name, uuid_t &out_uuid,
                          int client_fd, const std::string &missing_reason, const std::string &invalid_reason);

public:

    CommandHandler();
    ~CommandHandler();

    void register_client_fd(int fd);
    void unregister_client_fd(int fd);
    void disconnect_client_session(int fd);

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