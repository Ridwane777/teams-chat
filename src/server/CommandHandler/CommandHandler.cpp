#include "../../../include/server/CommandHandler/CommandHandler.hpp"

void CommandHandler::handle_command(int client_fd, const std::string &command)
{
    auto frame = _protocol.parse(command);
    if (!frame) {
        std::cerr << "Failed to parse command: invalid frame" << std::endl;
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
