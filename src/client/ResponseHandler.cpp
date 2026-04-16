#include "../../include/client/ResponseHandler.hpp"
#include <iostream>
extern "C" {
#include "../../libs/myteams/logging_client.h"
}

std::string ResponseHandler::_selfUuid = "";
std::string ResponseHandler::_pendingCommand = "";

void ResponseHandler::setPendingCommand(const std::string &cmd)
{
    _pendingCommand = cmd;
}

void ResponseHandler::execute(const std::string &opcode,
    const std::map<std::string, std::string> &fields)
{
    if (opcode == "ERR")
        handle_error(fields);
    else if (opcode == "EVT")
        handle_event(fields);
    else if (opcode == "OK")
        handle_ok(fields);
}

void ResponseHandler::handle_error(const std::map<std::string, std::string> &fields)
{
    std::string code = fields.count("code") ? fields.at("code") : "";
    std::string reason = fields.count("reason") ? fields.at("reason") : "";
    std::string uuid = fields.count("uuid") ? fields.at("uuid") : "unknown";

    if (code == "401")
        client_error_unauthorized();
    else if (code == "404") {
        if (reason == "team_not_found")
            client_error_unknown_team(uuid.c_str());
        else if (reason == "channel_not_found")
            client_error_unknown_channel(uuid.c_str());
        else if (reason == "thread_not_found")
            client_error_unknown_thread(uuid.c_str());
        else if (reason == "user_not_found")
            client_error_unknown_user(uuid.c_str());
    } else if (code == "409")
        client_error_already_exist();
}

void ResponseHandler::handle_event(const std::map<std::string, std::string> &fields)
{
    std::string event = fields.count("event") ? fields.at("event") : "";

    if (event == "USER_ONLINE")
        client_event_logged_in(fields.at("uuid").c_str(), fields.at("name").c_str());
    else if (event == "USER_OFFLINE")
        client_event_logged_out(fields.at("uuid").c_str(), fields.at("name").c_str());
    else if (event == "MSG_RECEIVED")
        client_event_private_message_received(fields.at("from").c_str(),
        fields.at("body").c_str());
    else if (event == "TEAM_CREATED")
        client_event_team_created(fields.at("uuid").c_str(),
        fields.at("name").c_str(), fields.at("description").c_str());
    else if (event == "CHANNEL_CREATED")
        client_event_channel_created(fields.at("uuid").c_str(),
        fields.at("name").c_str(), fields.at("description").c_str());
    else if (event == "THREAD_CREATED")
        client_event_thread_created(fields.at("uuid").c_str(),
        fields.at("creator").c_str(),
        (time_t)std::stoll(fields.at("timestamp")),
        fields.at("title").c_str(), fields.at("body").c_str());
    else if (event == "COMMENT_CREATED")
        client_event_thread_reply_received(fields.at("team").c_str(),
        fields.at("thread").c_str(), fields.at("author").c_str(), fields.at("body").c_str());
}

void ResponseHandler::handle_ok(const std::map<std::string, std::string> &fields)
{
    if (fields.count("count"))
        return;

    if (_pendingCommand == "/login" && fields.count("logged_in")) {
        if (_selfUuid.empty())
            _selfUuid = fields.at("uuid");
        client_event_logged_in(fields.at("uuid").c_str(), fields.at("name").c_str());
        return;
    }

    if (_pendingCommand == "/logout") {
        return;
    }

    if (_pendingCommand == "/users" && fields.count("logged_in")) {
        client_print_users(fields.at("uuid").c_str(), fields.at("name").c_str(),
            std::stoi(fields.at("logged_in")));
        return;
    }

    if (_pendingCommand == "/user" && fields.count("logged_in")) {
        client_print_user(fields.at("uuid").c_str(), fields.at("name").c_str(),
            std::stoi(fields.at("logged_in")));
        return;
    }

    if (_pendingCommand == "/send") {
        return;
    }

    if (_pendingCommand == "/messages" && fields.count("body") && fields.count("timestamp")) {
        client_private_message_print_messages(
            fields.at("from").c_str(),
            (time_t)std::stoll(fields.at("timestamp")),
            fields.at("body").c_str());
        return;
    }

    if (_pendingCommand == "/subscribe" && fields.count("team")) {
        client_print_subscribed(_selfUuid.c_str(), fields.at("team").c_str());
        return;
    }

    if (_pendingCommand == "/unsubscribe" && fields.count("team")) {
        client_print_unsubscribed(_selfUuid.c_str(), fields.at("team").c_str());
        return;
    }

    if (_pendingCommand == "/subscribed_teams" && fields.count("description")) {
        client_print_teams(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/subscribed_users" && fields.count("logged_in")) {
        client_print_users(fields.at("uuid").c_str(), fields.at("name").c_str(),
            std::stoi(fields.at("logged_in")));
        return;
    }

    if (_pendingCommand == "/create_team" && fields.count("creator") && !fields.count("title")) {
        client_print_team_created(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/create_channel" && fields.count("team") && !fields.count("title")) {
        client_print_channel_created(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/create_thread" && fields.count("title")) {
        client_print_thread_created(fields.at("uuid").c_str(), fields.at("creator").c_str(),
            (time_t)std::stoll(fields.at("timestamp")),
            fields.at("title").c_str(), fields.at("body").c_str());
        return;
    }

    if (_pendingCommand == "/create_reply" && fields.count("author")) {
        client_print_reply_created(fields.at("thread").c_str(), fields.at("author").c_str(),
            (time_t)std::stoll(fields.at("timestamp")), fields.at("body").c_str());
        return;
    }

    if (_pendingCommand == "/list_teams" && fields.count("description")) {
        client_print_teams(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/list_channels" && fields.count("team")) {
        client_team_print_channels(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/list_threads" && fields.count("title")) {
        client_channel_print_threads(fields.at("uuid").c_str(), fields.at("creator").c_str(),
            (time_t)std::stoll(fields.at("timestamp")),
            fields.at("title").c_str(), fields.at("body").c_str());
        return;
    }

    if (_pendingCommand == "/list_replies" && fields.count("author")) {
        client_thread_print_replies(fields.at("thread").c_str(), fields.at("author").c_str(),
            (time_t)std::stoll(fields.at("timestamp")), fields.at("body").c_str());
        return;
    }

    if (_pendingCommand == "/info_user" && fields.count("logged_in")) {
        client_print_user(fields.at("uuid").c_str(), fields.at("name").c_str(),
            std::stoi(fields.at("logged_in")));
        return;
    }

    if (_pendingCommand == "/info_team" && fields.count("creator")) {
        client_print_team(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/info_channel" && fields.count("team") && !fields.count("title")) {
        client_print_channel(fields.at("uuid").c_str(), fields.at("name").c_str(),
            fields.at("description").c_str());
        return;
    }

    if (_pendingCommand == "/info_thread" && fields.count("title")) {
        client_print_thread(fields.at("uuid").c_str(), fields.at("creator").c_str(),
            (time_t)std::stoll(fields.at("timestamp")),
            fields.at("title").c_str(), fields.at("body").c_str());
        return;
    }
}
