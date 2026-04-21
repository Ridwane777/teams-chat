#include "../../../include/server/CommandHandler/CommandHandler.hpp"
#include <algorithm>

CommandHandler::CommandHandler()
{
    commands["LOG"] = [this](int fd, Frame &f) { handle_login(fd, f); };
    commands["OUT"] = [this](int fd, Frame &f) { handle_logout(fd, f); };
    commands["ULS"] = [this](int fd, Frame &f) { handle_users_list(fd, f); };
    commands["UGT"] = [this](int fd, Frame &f) { handle_user_get(fd, f); };
    commands["SND"] = [this](int fd, Frame &f) { handle_msg_send(fd, f); };
    commands["MLS"] = [this](int fd, Frame &f) { handle_msg_list(fd, f); };
    commands["TCR"] = [this](int fd, Frame &f) { handle_team_create(fd, f); };
    commands["TSB"] = [this](int fd, Frame &f) { handle_team_sub(fd, f); };
    commands["TUS"] = [this](int fd, Frame &f) { handle_team_unsub(fd, f); };
    commands["TSD"] = [this](int fd, Frame &f) { handle_team_subbed(fd, f); };
    commands["CCR"] = [this](int fd, Frame &f) { handle_chan_create(fd, f); };
    commands["HCR"] = [this](int fd, Frame &f) { handle_thread_create(fd, f); };
    commands["OCR"] = [this](int fd, Frame &f) { handle_comment_create(fd, f); };
    commands["LST"] = [this](int fd, Frame &f) { handle_list(fd, f); };
    commands["INF"] = [this](int fd, Frame &f) { handle_info(fd, f); };
    commands["USE"] = [this](int fd, Frame &f) { handle_use(fd, f); };
}

CommandHandler::~CommandHandler() = default;

void CommandHandler::save_state(const std::string &filepath)
{
    _server_state.save(filepath);
}

void CommandHandler::load_state(const std::string &filepath)
{
    _server_state.load(filepath);
}

void CommandHandler::register_client_fd(int fd)
{
    _connected_clients.insert(fd);
}

void CommandHandler::unregister_client_fd(int fd)
{
    _connected_clients.erase(fd);
    _contexts.erase(fd);
}

void CommandHandler::send_frame(int fd, const std::string &frame)
{
    write(fd, frame.c_str(), frame.size());
}

void CommandHandler::broadcast_frame(const std::string &frame)
{
    for (int fd : _connected_clients)
        send_frame(fd, frame);
}

void CommandHandler::broadcast_team_event(const uuid_t &team_uuid, const std::string &frame)
{
    for (int fd : _connected_clients) {
        User *u = _server_state.findUserByFd(fd);
        if (!u || !u->is_logged_in)
            continue;
        if (_server_state.subscriptionExists(u->uuid, team_uuid))
            send_frame(fd, frame);
    }
}

User *CommandHandler::require_logged_user(int client_fd)
{
    User *u = _server_state.findUserByFd(client_fd);
    if (!u || !u->is_logged_in) {
        send_frame(client_fd, _protocol.err(ErrCode::UNAUTHORIZED, "not_logged_in"));
        return nullptr;
    }
    return u;
}

bool CommandHandler::parse_uuid_field(const Frame &frame, const std::string &field_name, uuid_t &out_uuid,
                                      int client_fd, const std::string &missing_reason, const std::string &invalid_reason)
{
    if (!frame.has(field_name)) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, missing_reason));
        return false;
    }
    if (uuid_parse(frame.get(field_name).c_str(), out_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, invalid_reason));
        return false;
    }
    return true;
}

void CommandHandler::disconnect_client_session(int fd)
{
    User *u = _server_state.findUserByFd(fd);
    if (!u || !u->is_logged_in)
        return;

    std::string user_uuid = uuidToString(u->uuid);
    std::string user_name = u->name;
    u->is_logged_in = false;
    u->fd = -1;
    server_event_user_logged_out(user_uuid.c_str());
    broadcast_frame(_protocol.evt(EventType::USER_OFFLINE, {{"uuid", user_uuid}, {"name", user_name}}));
}

void CommandHandler::handle_logout_all_clients()
{
    for (int fd : _connected_clients) {
        User *u = _server_state.findUserByFd(fd);
        if (!u || !u->is_logged_in)
            continue;

        std::string user_uuid = uuidToString(u->uuid);
        std::string user_name = u->name;
        u->is_logged_in = false;
        u->fd = -1;
        server_event_user_logged_out(user_uuid.c_str());
        broadcast_frame(_protocol.evt(EventType::USER_OFFLINE, {{"uuid", user_uuid}, {"name", user_name}}));
    }
}

void CommandHandler::handle_command(int client_fd, const std::string &command)
{
    auto frame = _protocol.parse(command);
    if (!frame) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "invalid_command"));
        return;
    }
    execute_command(client_fd, *frame);
}

void CommandHandler::execute_command(int client_fd, Frame &frame)
{
    auto cmd = commands.find(frame.opcode);

    if (cmd == commands.end()) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "unknown_command"));
        return;
    }
    if (frame.opcode != std::string(ClientOp::LOGIN) && !require_logged_user(client_fd))
        return;
    cmd->second(client_fd, frame);
}

void CommandHandler::handle_login(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    if (self && self->is_logged_in) {
        send_frame(client_fd, _protocol.err(ErrCode::CONFLICT, "already_logged_in"));
        return;
    }

    std::string name = frame.get("name");
    if (!_validate_login_name(name, client_fd))
        return;

    User *existing_user = _server_state.findUserByName(name);
    if (existing_user)
        _login_existing_user(existing_user, client_fd);
    else
        _login_new_user(name, client_fd);
}

bool CommandHandler::_validate_login_name(const std::string &name, int client_fd)
{
    if (name.empty() || name.size() > 32) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "invalid_name"));
        return false;
    }
    return true;
}

void CommandHandler::_login_existing_user(User *user, int client_fd)
{
    if (user->is_logged_in) {
        send_frame(client_fd, _protocol.err(ErrCode::CONFLICT, "already_logged_in"));
        return;
    }
    user->fd = client_fd;
    user->is_logged_in = true;
    server_event_user_logged_in(uuidToString(user->uuid).c_str());
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(user->uuid)}, {"name", user->name}, {"logged_in", "1"}}));
    broadcast_frame(_protocol.evt(EventType::USER_ONLINE, {{"uuid", uuidToString(user->uuid)}, {"name", user->name}}));
}

void CommandHandler::_login_new_user(const std::string &name, int client_fd)
{
    User &new_user = _server_state.createUser(name, client_fd);
    new_user.is_logged_in = true;
    std::string user_uuid = uuidToString(new_user.uuid);
    server_event_user_logged_in(user_uuid.c_str());
    send_frame(client_fd, _protocol.ok({{"uuid", user_uuid}, {"name", new_user.name}, {"logged_in", "1"}}));
    broadcast_frame(_protocol.evt(EventType::USER_ONLINE, {{"uuid", user_uuid}, {"name", new_user.name}}));
}

void CommandHandler::handle_logout(int client_fd, Frame &frame)
{
    (void)frame;
    User *user = require_logged_user(client_fd);
    if (!user)
        return;

    std::string user_uuid = uuidToString(user->uuid);
    std::string user_name = user->name;
    user->is_logged_in = false;
    user->fd = -1;
    server_event_user_logged_out(user_uuid.c_str());
    send_frame(client_fd, _protocol.ok({}));
    broadcast_frame(_protocol.evt(EventType::USER_OFFLINE, {{"uuid", user_uuid}, {"name", user_name}}));
}

void CommandHandler::handle_users_list(int client_fd, Frame &frame)
{
    (void)frame;
    std::string response;

    for (const User &user : _server_state.users()) {
        response += _protocol.ok({{"uuid", uuidToString(user.uuid)}, {"name", user.name}, {"logged_in", user.is_logged_in ? "1" : "0"}});
    }
    response += _protocol.ok({{"count", std::to_string(_server_state.users().size())}});
    send_frame(client_fd, response);
}

void CommandHandler::handle_user_get(int client_fd, Frame &frame)
{
    uuid_t uuid;

    if (!parse_uuid_field(frame, "uuid", uuid, client_fd, "missing_uuid", "invalid_uuid"))
        return;

    User *user = _server_state.findUserByUuid(uuid);
    if (!user) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "user_not_found"));
        return;
    }
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(user->uuid)},
                                        {"name", user->name},
                                        {"logged_in", user->is_logged_in ? "1" : "0"}}));
}

void CommandHandler::handle_msg_send(int client_fd, Frame &frame)
{
    User *sender = require_logged_user(client_fd);
    if (!sender)
        return;

    uuid_t recipient_uuid;
    if (!_validate_message(frame, sender, client_fd, recipient_uuid))
        return;

    User *recipient = _server_state.findUserByUuid(recipient_uuid);
    if (!recipient) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "user_not_found"));
        return;
    }

    Message &message = _server_state.createMessage(frame.get("body"), sender->uuid, recipient_uuid);
    _send_message(message, recipient, sender, client_fd);
}

bool CommandHandler::_validate_message(const Frame &frame, User *sender, int client_fd, uuid_t &recipient_uuid)
{
    if (!parse_uuid_field(frame, "to", recipient_uuid, client_fd, "missing_to", "invalid_uuid"))
        return false;
    std::string body = frame.get("body");
    if (body.empty() || body.size() > 512) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "body_too_long"));
        return false;
    }
    if (uuid_compare(sender->uuid, recipient_uuid) == 0) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "cannot_message_self"));
        return false;
    }
    return true;
}

void CommandHandler::_send_message(Message &message, User *recipient, User *sender, int client_fd)
{
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(message.uuid)}, {"timestamp", std::to_string(message.timestamp)}}));
    if (recipient->is_logged_in && recipient->fd >= 0) {
        send_frame(recipient->fd, _protocol.evt(EventType::MSG_RECEIVED,
                                                {{"from", uuidToString(sender->uuid)},
                                                 {"to", uuidToString(recipient->uuid)},
                                                 {"body", message.body},
                                                 {"timestamp", std::to_string(message.timestamp)}}));
    }
}

void CommandHandler::handle_msg_list(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    uuid_t other_uuid;
    std::string response;

    if (!self)
        return;
    if (!parse_uuid_field(frame, "with", other_uuid, client_fd, "missing_with", "invalid_uuid"))
        return;
    if (!_server_state.findUserByUuid(other_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "user_not_found"));
        return;
    }

    std::vector<Message *> conv = _server_state.getConversation(self->uuid, other_uuid);
    for (Message *msg : conv) {
        response += _protocol.ok({{"uuid", uuidToString(msg->uuid)},
                                  {"from", uuidToString(msg->sender_uuid)},
                                  {"to", uuidToString(msg->receiver_uuid)},
                                  {"body", msg->body},
                                  {"timestamp", std::to_string(msg->timestamp)}});
    }
    response += _protocol.ok({{"count", std::to_string(conv.size())}});
    send_frame(client_fd, response);
}

void CommandHandler::handle_team_create(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    if (!self)
        return;

    std::string name = frame.get("name");
    if (name.empty() || name.size() > 32) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "name_too_long"));
        return;
    }
    if (_server_state.findTeamByName(name)) {
        send_frame(client_fd, _protocol.err(ErrCode::CONFLICT, "team_exists"));
        return;
    }

    Team &team = _server_state.createTeam(name, frame.get("description"), self->uuid);
    if (!_server_state.subscriptionExists(self->uuid, team.uuid))
        _server_state.createSubscription(self->uuid, team.uuid);

    std::string team_uuid = uuidToString(team.uuid);
    std::string creator_uuid = uuidToString(team.creator_uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", team_uuid}, {"name", team.name}, {"description", team.description}, {"creator", creator_uuid}}));
    broadcast_frame(_protocol.evt(EventType::TEAM_CREATED, {{"uuid", team_uuid}, {"name", team.name}, {"description", team.description}, {"creator", creator_uuid}}));
}

void CommandHandler::handle_team_sub(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    uuid_t team_uuid;

    if (!self)
        return;
    if (!parse_uuid_field(frame, "team", team_uuid, client_fd, "missing_team", "invalid_uuid"))
        return;
    Team *team = _server_state.findTeamByUuid(team_uuid);
    if (!team) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "team_not_found"));
        return;
    }
    if (_server_state.subscriptionExists(self->uuid, team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::CONFLICT, "already_subscribed"));
        return;
    }
    _server_state.createSubscription(self->uuid, team_uuid);
    send_frame(client_fd, _protocol.ok({{"team", uuidToString(team_uuid)}}));
}

void CommandHandler::handle_team_unsub(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    uuid_t team_uuid;

    if (!self)
        return;
    if (!parse_uuid_field(frame, "team", team_uuid, client_fd, "missing_team", "invalid_uuid"))
        return;
    if (!_server_state.findTeamByUuid(team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "team_not_found"));
        return;
    }
    if (!_server_state.removeSubscription(self->uuid, team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "not_subscribed"));
        return;
    }
    send_frame(client_fd, _protocol.ok({{"team", uuidToString(team_uuid)}}));
}

void CommandHandler::handle_team_subbed(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    std::string response;

    if (!self)
        return;

    if (!frame.has("team")) {
        std::vector<Team *> teams = _server_state.getUserTeams(self->uuid);
        for (Team *team : teams) {
            response += _protocol.ok({{"uuid", uuidToString(team->uuid)},
                                      {"name", team->name},
                                      {"description", team->description}});
        }
        response += _protocol.ok({{"count", std::to_string(teams.size())}});
        send_frame(client_fd, response);
        return;
    }

    uuid_t team_uuid;
    if (!parse_uuid_field(frame, "team", team_uuid, client_fd, "missing_team", "invalid_uuid"))
        return;
    Team *team = _server_state.findTeamByUuid(team_uuid);
    if (!team) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "team_not_found"));
        return;
    }

    std::vector<User *> users = _server_state.getTeamMembers(team_uuid);
    for (User *u : users) {
        response += _protocol.ok({{"uuid", uuidToString(u->uuid)},
                                  {"name", u->name},
                                  {"logged_in", u->is_logged_in ? "1" : "0"}});
    }
    response += _protocol.ok({{"count", std::to_string(users.size())}});
    send_frame(client_fd, response);
}

void CommandHandler::handle_use(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    if (!self)
        return;
    
    ClientContext next;
    if (!frame.has("team") && !frame.has("channel") && !frame.has("thread")) {
        _contexts[client_fd] = next;
        send_frame(client_fd, _protocol.ok({{"team", "-"}, {"channel", "-"}, {"thread", "-"}}));
        return;
    }
    if ((frame.has("channel") && !frame.has("team")) || (frame.has("thread") && !frame.has("channel"))) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "invalid_context"));
        return;
    }

    if (frame.has("team") && !_validate_use_team(frame.get("team"), next.team, client_fd))
        return;
    if (frame.has("channel") && !_validate_use_channel(next.team, frame.get("channel"), next.channel, client_fd))
        return;
    if (frame.has("thread") && !_validate_use_thread(next.channel, frame.get("thread"), next.thread, client_fd))
        return;

    _contexts[client_fd] = next;
    send_frame(client_fd, _protocol.ok({{"team", next.team.empty() ? "-" : next.team},
                                        {"channel", next.channel.empty() ? "-" : next.channel},
                                        {"thread", next.thread.empty() ? "-" : next.thread}}));
}

bool CommandHandler::_validate_use_team(const std::string &team_str, std::string &out_team, int client_fd)
{
    uuid_t team_uuid;
    if (uuid_parse(team_str.c_str(), team_uuid) != 0 || !_server_state.findTeamByUuid(team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "team_not_found"));
        return false;
    }
    out_team = team_str;
    return true;
}

bool CommandHandler::_validate_use_channel(const std::string &team_str, const std::string &chan_str, std::string &out_chan, int client_fd)
{
    uuid_t channel_uuid;
    if (uuid_parse(chan_str.c_str(), channel_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "channel_not_found"));
        return false;
    }
    Channel *channel = _server_state.findChannelByUuid(channel_uuid);
    if (!channel || uuidToString(channel->team_uuid) != team_str) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "channel_not_found"));
        return false;
    }
    out_chan = chan_str;
    return true;
}

bool CommandHandler::_validate_use_thread(const std::string &chan_str, const std::string &thread_str, std::string &out_thread, int client_fd)
{
    uuid_t thread_uuid;
    if (uuid_parse(thread_str.c_str(), thread_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "thread_not_found"));
        return false;
    }
    Thread *thread = _server_state.findThreadByUuid(thread_uuid);
    if (!thread || uuidToString(thread->channel_uuid) != chan_str) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "thread_not_found"));
        return false;
    }
    out_thread = thread_str;
    return true;
}

void CommandHandler::handle_chan_create(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    ClientContext &ctx = _contexts[client_fd];
    uuid_t team_uuid;

    if (!self)
        return;
    if (ctx.team.empty()) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "context_required"));
        return;
    }
    if (uuid_parse(ctx.team.c_str(), team_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "team_not_found"));
        return;
    }
    if (!_server_state.subscriptionExists(self->uuid, team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "not_subscribed"));
        return;
    }

    std::string name = frame.get("name");
    std::string description = frame.get("description");
    if (name.empty()) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "invalid_name"));
        return;
    }
    if (_server_state.findChannelByNameInTeam(name, team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::CONFLICT, "channel_exists"));
        return;
    }

    Channel &channel = _server_state.createChannel(name, description, team_uuid, self->uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(channel.uuid)},
                                        {"name", channel.name},
                                        {"description", channel.description},
                                        {"team", uuidToString(channel.team_uuid)}}));
    broadcast_team_event(team_uuid, _protocol.evt(EventType::CHANNEL_CREATED,
                                                  {{"uuid", uuidToString(channel.uuid)},
                                                   {"name", channel.name},
                                                   {"description", channel.description},
                                                   {"team", uuidToString(channel.team_uuid)}}));
}

void CommandHandler::handle_thread_create(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    ClientContext &ctx = _contexts[client_fd];
    uuid_t team_uuid;
    uuid_t channel_uuid;

    if (!self)
        return;
    if (ctx.team.empty() || ctx.channel.empty()) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "context_required"));
        return;
    }
    if (uuid_parse(ctx.team.c_str(), team_uuid) != 0 || uuid_parse(ctx.channel.c_str(), channel_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "invalid_context"));
        return;
    }
    if (!_server_state.subscriptionExists(self->uuid, team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "not_subscribed"));
        return;
    }

    Thread &thread = _server_state.createThread(frame.get("title"), frame.get("body"), channel_uuid, self->uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(thread.uuid)},
                                        {"title", thread.title},
                                        {"body", thread.body},
                                        {"channel", uuidToString(thread.channel_uuid)},
                                        {"creator", uuidToString(thread.creator_uuid)},
                                        {"timestamp", std::to_string(thread.timestamp)}}));
    broadcast_team_event(team_uuid, _protocol.evt(EventType::THREAD_CREATED,
                                                  {{"uuid", uuidToString(thread.uuid)},
                                                   {"title", thread.title},
                                                   {"body", thread.body},
                                                   {"channel", uuidToString(thread.channel_uuid)},
                                                   {"creator", uuidToString(thread.creator_uuid)},
                                                   {"timestamp", std::to_string(thread.timestamp)}}));
}

void CommandHandler::handle_comment_create(int client_fd, Frame &frame)
{
    User *self = require_logged_user(client_fd);
    ClientContext &ctx = _contexts[client_fd];
    uuid_t team_uuid;
    uuid_t thread_uuid;

    if (!self)
        return;
    if (ctx.team.empty() || ctx.channel.empty() || ctx.thread.empty()) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "context_required"));
        return;
    }
    if (uuid_parse(ctx.team.c_str(), team_uuid) != 0 || uuid_parse(ctx.thread.c_str(), thread_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::BAD_REQUEST, "invalid_context"));
        return;
    }
    if (!_server_state.subscriptionExists(self->uuid, team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::FORBIDDEN, "not_subscribed"));
        return;
    }

    Comment &comment = _server_state.createComment(frame.get("body"), thread_uuid, self->uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(comment.uuid)},
                                        {"body", comment.body},
                                        {"thread", uuidToString(comment.thread_uuid)},
                                        {"author", uuidToString(comment.author_uuid)},
                                        {"timestamp", std::to_string(comment.timestamp)}}));
    broadcast_team_event(team_uuid, _protocol.evt(EventType::COMMENT_CREATED,
                                                  {{"uuid", uuidToString(comment.uuid)},
                                                   {"body", comment.body},
                                                   {"team", uuidToString(team_uuid)},
                                                   {"thread", uuidToString(comment.thread_uuid)},
                                                   {"author", uuidToString(comment.author_uuid)},
                                                   {"timestamp", std::to_string(comment.timestamp)}}));
}

void CommandHandler::handle_list(int client_fd, Frame &frame)
{
    (void)frame;
    User *self = require_logged_user(client_fd);
    if (!self)
        return;

    ClientContext &ctx = _contexts[client_fd];
    if (ctx.team.empty())
        _list_teams(client_fd);
    else if (ctx.channel.empty())
        _list_channels(client_fd, ctx.team);
    else if (ctx.thread.empty())
        _list_threads(client_fd, ctx.channel);
    else
        _list_comments(client_fd, ctx.thread);
}

void CommandHandler::_list_teams(int client_fd)
{
    std::string response;
    for (Team &team : _server_state.teams())
        response += _protocol.ok({{"uuid", uuidToString(team.uuid)}, {"name", team.name}, {"description", team.description}});
    response += _protocol.ok({{"count", std::to_string(_server_state.teams().size())}});
    send_frame(client_fd, response);
}

void CommandHandler::_list_channels(int client_fd, const std::string &team_str)
{
    uuid_t team_uuid;
    if (uuid_parse(team_str.c_str(), team_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "resource_not_found"));
        return;
    }
    std::string response;
    std::vector<Channel *> channels = _server_state.getTeamChannels(team_uuid);
    for (Channel *ch : channels)
        response += _protocol.ok({{"uuid", uuidToString(ch->uuid)}, {"name", ch->name}, {"description", ch->description}, {"team", uuidToString(ch->team_uuid)}});
    response += _protocol.ok({{"count", std::to_string(channels.size())}});
    send_frame(client_fd, response);
}

void CommandHandler::_list_threads(int client_fd, const std::string &channel_str)
{
    uuid_t channel_uuid;
    if (uuid_parse(channel_str.c_str(), channel_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "resource_not_found"));
        return;
    }
    std::string response;
    std::vector<Thread *> threads = _server_state.getChannelThreads(channel_uuid);
    for (Thread *th : threads)
        response += _protocol.ok({{"uuid", uuidToString(th->uuid)}, {"title", th->title}, {"body", th->body}, {"channel", uuidToString(th->channel_uuid)}, {"creator", uuidToString(th->creator_uuid)}, {"timestamp", std::to_string(th->timestamp)}});
    response += _protocol.ok({{"count", std::to_string(threads.size())}});
    send_frame(client_fd, response);
}

void CommandHandler::_list_comments(int client_fd, const std::string &thread_str)
{
    uuid_t thread_uuid;
    if (uuid_parse(thread_str.c_str(), thread_uuid) != 0) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "resource_not_found"));
        return;
    }
    std::string response;
    std::vector<Comment *> comments = _server_state.getThreadComments(thread_uuid);
    for (Comment *co : comments)
        response += _protocol.ok({{"uuid", uuidToString(co->uuid)}, {"body", co->body}, {"thread", uuidToString(co->thread_uuid)}, {"author", uuidToString(co->author_uuid)}, {"timestamp", std::to_string(co->timestamp)}});
    response += _protocol.ok({{"count", std::to_string(comments.size())}});
    send_frame(client_fd, response);
}

void CommandHandler::handle_info(int client_fd, Frame &frame)
{
    (void)frame;
    User *self = require_logged_user(client_fd);
    if (!self)
        return;

    ClientContext &ctx = _contexts[client_fd];
    if (ctx.team.empty())
        _info_self(client_fd, self);
    else if (ctx.channel.empty())
        _info_team(client_fd, ctx.team);
    else if (ctx.thread.empty())
        _info_channel(client_fd, ctx.channel);
    else
        _info_thread(client_fd, ctx.thread);
}

void CommandHandler::_info_self(int client_fd, User *self)
{
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(self->uuid)}, {"name", self->name}, {"logged_in", self->is_logged_in ? "1" : "0"}}));
}

void CommandHandler::_info_team(int client_fd, const std::string &team_str)
{
    uuid_t team_uuid;
    if (uuid_parse(team_str.c_str(), team_uuid) != 0 || !_server_state.findTeamByUuid(team_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "resource_not_found"));
        return;
    }
    Team *team = _server_state.findTeamByUuid(team_uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(team->uuid)}, {"name", team->name}, {"description", team->description}, {"creator", uuidToString(team->creator_uuid)}}));
}

void CommandHandler::_info_channel(int client_fd, const std::string &chan_str)
{
    uuid_t channel_uuid;
    if (uuid_parse(chan_str.c_str(), channel_uuid) != 0 || !_server_state.findChannelByUuid(channel_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "resource_not_found"));
        return;
    }
    Channel *channel = _server_state.findChannelByUuid(channel_uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(channel->uuid)}, {"name", channel->name}, {"description", channel->description}, {"team", uuidToString(channel->team_uuid)}}));
}

void CommandHandler::_info_thread(int client_fd, const std::string &thread_str)
{
    uuid_t thread_uuid;
    if (uuid_parse(thread_str.c_str(), thread_uuid) != 0 || !_server_state.findThreadByUuid(thread_uuid)) {
        send_frame(client_fd, _protocol.err(ErrCode::NOT_FOUND, "resource_not_found"));
        return;
    }
    Thread *thread = _server_state.findThreadByUuid(thread_uuid);
    send_frame(client_fd, _protocol.ok({{"uuid", uuidToString(thread->uuid)}, {"title", thread->title}, {"body", thread->body}, {"channel", uuidToString(thread->channel_uuid)}, {"creator", uuidToString(thread->creator_uuid)}, {"timestamp", std::to_string(thread->timestamp)}}));
}
