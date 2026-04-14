#include "../../../include/server/CommandHandler/CommandHandler.hpp"

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

void CommandHandler::handle_login(int client_fd)
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
            write(client_fd, _protocol.ok({{"user_uuid", std::string(user->uuid, user->uuid + 16)}}).c_str(),
                  _protocol.ok({{"user_uuid", std::string(user->uuid, user->uuid + 16)}}).size());
            
        }
        return;
    }
}
