#include "../../../include/server/CommandHandler/CommandHandler.hpp"

CommandHandler::CommandHandler()
{
    commands["LOG"] = [this](int fd, Frame &f) { handle_login(fd, f); };
    commands["OUT"] = [this](int fd, Frame &f) { handle_logout(fd, f); };
    commands["ULS"] = [this](int fd, Frame &f) { handle_users_list(fd, f); };
}

void CommandHandler::handle_command(int client_fd, const std::string &command)
{
    auto frame = _protocol.parse(command);
    if (!frame) {
        std::cout << "Failed to parse command: invalid frame" << std::endl;
        _protocol.err(ErrCode::BAD_REQUEST, "invalid_command");
        return;
    }

    execute_command(client_fd, *frame);
}

void CommandHandler::execute_command(int client_fd, Frame &frame)
{
    (void)client_fd;
    (void)frame;
}

void CommandHandler::handle_login(int client_fd, Frame &frame)
{
    User *user = _server_state.findUserByFd(client_fd);
    if (user) {
        if (user->is_logged_in) {
            std::cout << "User already logged in: " << user->name << std::endl;
            write(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "already_logged_in").c_str(),
                  _protocol.err(ErrCode::BAD_REQUEST, "already_logged_in").size());
        } else {
            char user_uuid[37] = {0};

            user->is_logged_in = true;
            uuid_unparse_lower(user->uuid, user_uuid);
            server_event_user_logged_in(user_uuid);
            std::cout << "User logged in: " << user->name << std::endl;
            const std::string response = _protocol.ok({{"uuid", user_uuid}, {"name", user->name}, {"logged_in", "1"}});
            write(client_fd, response.c_str(), response.size());
            
        }
        return;
    } else {
        std::string name = frame.get("name");
        if (name.empty()) {
            std::cout << "Login failed: missing name field" << std::endl;
            write(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "missing_name").c_str(),
                  _protocol.err(ErrCode::BAD_REQUEST, "missing_name").size());
            return;
        }
        User &new_user = _server_state.createUser(name, client_fd);
        char user_uuid[37] = {0};
        uuid_unparse_lower(new_user.uuid, user_uuid);
        std::cout << "New user created and logged in: " << new_user.name << std::endl;
        server_event_user_created(user_uuid, new_user.name.c_str());
        new_user.is_logged_in = true;
        server_event_user_logged_in(user_uuid);
        const std::string response = "New user created and logged_in: " + _protocol.ok({{"uuid", user_uuid}, {"name", new_user.name}, {"logged_in", "1"}});
        write(client_fd, response.c_str(), response.size());
    }
}

void CommandHandler::handle_logout(int client_fd, Frame &frame)
{
    (void)frame;
    User *user = _server_state.findUserByFd(client_fd);
    if (!user || !user->is_logged_in) {
        std::cout << "Logout failed: no user logged in on this fd" << std::endl;
        write(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "not_logged_in").c_str(),
              _protocol.err(ErrCode::BAD_REQUEST, "not_logged_in").size());
        return;
    }
    char user_uuid[37] = {0};
    uuid_unparse_lower(user->uuid, user_uuid);
    user->is_logged_in = false;
    server_event_user_logged_out(user_uuid);
    std::cout << "User logged out: " << user->name << std::endl;
    const std::string response = _protocol.ok({});
    write(client_fd, response.c_str(), response.size());
}

void CommandHandler::handle_users_list(int client_fd, Frame &frame)
{
    (void)frame;
    std::string response;
    for (const User &user : _server_state.users()) {
        response += _protocol.ok({{"uuid", uuidToString(user.uuid)}, {"name", user.name}, {"logged_in", user.is_logged_in ? "1" : "0"}});
    }
    response += _protocol.ok({{"count", std::to_string(_server_state.users().size())}});
    write(client_fd, response.c_str(), response.size());
}
