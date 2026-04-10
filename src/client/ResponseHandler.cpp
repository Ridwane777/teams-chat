#include "../../include/client/ResponseHandler.hpp"

extern "C" {
#include "../../libs/myteams/logging_client.h"
}

std::map<std::string, ResponseHandler::handler_t> ResponseHandler::_responses;

void ResponseHandler::initMap()
{
    _responses["200"] = &handleLoggedIn;
    _responses["201"] = &handleLoggedOut;
    _responses["202"] = &handlePrivateMessageReceived;
    _responses["203"] = &handlePrintUsers;
    _responses["204"] = &handlePrintTeams;
    _responses["401"] = &handleUnknownTeam;
    _responses["402"] = &handleUnknownChannel;
    _responses["403"] = &handleUnknownThread;
    _responses["404"] = &handleUnknownUser;
    _responses["405"] = &handleUnauthorized;
    _responses["406"] = &handleAlreadyExist;
}

void ResponseHandler::execute(const std::string &code, const std::vector<std::string> &args)
{
    if (_responses.empty())
        initMap();

    if (_responses.find(code) != _responses.end()) {
        _responses[code](args);
    } else {
        std::cout << "Unknown server code: " << code << std::endl;
        for (const auto &arg : args) {
            std::cout << "  Arg: " << arg << std::endl;
        }
    }
}

void ResponseHandler::handleLoggedIn(const std::vector<std::string> &args)
{
    if (args.size() == 2)
        client_event_logged_in(args[0].c_str(), args[1].c_str());
}

void ResponseHandler::handleLoggedOut(const std::vector<std::string> &args)
{
    if (args.size() == 2)
        client_event_logged_out(args[0].c_str(), args[1].c_str());
}

void ResponseHandler::handlePrivateMessageReceived(const std::vector<std::string> &args)
{
    if (args.size() == 2)
        client_event_private_message_received(args[0].c_str(), args[1].c_str());
}

void ResponseHandler::handlePrintUsers(const std::vector<std::string> &args)
{
    if (args.size() == 3)
        client_print_users(args[0].c_str(), args[1].c_str(), std::stoi(args[2]));
}

void ResponseHandler::handlePrintTeams(const std::vector<std::string> &args)
{
    if (args.size() == 3)
        client_print_teams(args[0].c_str(), args[1].c_str(), args[2].c_str());
}

void ResponseHandler::handleUnknownTeam(const std::vector<std::string> &args)
{
    if (args.size() == 1)
        client_error_unknown_team(args[0].c_str());
}

void ResponseHandler::handleUnknownChannel(const std::vector<std::string> &args)
{
    if (args.size() == 1)
        client_error_unknown_channel(args[0].c_str());
}

void ResponseHandler::handleUnknownThread(const std::vector<std::string> &args)
{
    if (args.size() == 1)
        client_error_unknown_thread(args[0].c_str());
}

void ResponseHandler::handleUnknownUser(const std::vector<std::string> &args)
{
    if (args.size() == 1)
        client_error_unknown_user(args[0].c_str());
}

void ResponseHandler::handleUnauthorized(const std::vector<std::string> &args)
{
    (void)args;
    client_error_unauthorized();
}

void ResponseHandler::handleAlreadyExist(const std::vector<std::string> &args)
{
    (void)args;
    client_error_already_exist();
}
