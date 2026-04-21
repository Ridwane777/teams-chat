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
    void broadcast_frame_except(int exclude_fd, const std::string &frame);
    void broadcast_team_event(const uuid_t &team_uuid, const std::string &frame);
    User *require_logged_user(int client_fd);
    bool parse_uuid_field(const Frame &frame, const std::string &field_name, uuid_t &out_uuid,
                          int client_fd, const std::string &missing_reason, const std::string &invalid_reason);

    // Login helpers
    bool _validate_login_name(const std::string &name, int client_fd);
    void _login_existing_user(User *user, int client_fd);
    void _login_new_user(const std::string &name, int client_fd);

    // Use context helpers
    bool _validate_use_team(const std::string &team_str, std::string &out_team, int client_fd);
    bool _validate_use_channel(const std::string &team_str, const std::string &chan_str, std::string &out_chan, int client_fd);
    bool _validate_use_thread(const std::string &chan_str, const std::string &thread_str, std::string &out_thread, int client_fd);

    // List helpers
    void _list_teams(int client_fd);
    void _list_channels(int client_fd, const std::string &team_str);
    void _list_threads(int client_fd, const std::string &channel_str);
    void _list_comments(int client_fd, const std::string &thread_str);

    // Info helpers
    void _info_self(int client_fd, User *self);
    void _info_team(int client_fd, const std::string &team_str);
    void _info_channel(int client_fd, const std::string &chan_str);
    void _info_thread(int client_fd, const std::string &thread_str);

    // Message helpers
    bool _validate_message(const Frame &frame, User *sender, int client_fd, uuid_t &recipient_uuid);
    void _send_message(Message &message, User *recipient, User *sender, int client_fd);

public:

    CommandHandler();
    ~CommandHandler();

    void save_state(const std::string &filepath);
    void load_state(const std::string &filepath);

    void register_client_fd(int fd);
    void unregister_client_fd(int fd);
    void disconnect_client_session(int fd);
    void handle_logout_all_clients();

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