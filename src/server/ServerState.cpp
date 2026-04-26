#include "../../include/server/ServerState.hpp"
extern "C" {
#include "../../libs/myteams/logging_server.h"
}
#include <algorithm>
#include <stdexcept>

void ServerState::_generateUuid(uuid_t &uuid)
{
    uuid_generate(uuid);
}

template <typename T>
bool ServerState::_removeByUuid(std::vector<T> &vec, const uuid_t &uuid)
{
    auto it = std::find_if(vec.begin(), vec.end(),
                           [&uuid](const T &e)
                           { return uuid_compare(e.uuid, uuid) == 0; });

    if (it == vec.end())
        return false;
    vec.erase(it);
    return true;
}

User &ServerState::createUser(const std::string &name, int fd)
{
    uuid_t generated_uuid;

    _generateUuid(generated_uuid);
    _users.emplace_back(generated_uuid, name, fd);
    const std::string user_uuid = uuidToString(_users.back().uuid);
    server_event_user_created(user_uuid.c_str(), _users.back().name.c_str());
    return _users.back();
}

User *ServerState::findUserByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_users.begin(), _users.end(),
                           [&uuid](const User &u)
                           { return uuid_compare(u.uuid, uuid) == 0; });
    return it != _users.end() ? &(*it) : nullptr;
}

User *ServerState::findUserByName(const std::string &name)
{
    auto it = std::find_if(_users.begin(), _users.end(),
                           [&name](const User &u)
                           { return u.name == name; });
    return it != _users.end() ? &(*it) : nullptr;
}

User *ServerState::findUserByFd(int fd)
{
    auto it = std::find_if(_users.begin(), _users.end(),
                           [fd](const User &u)
                           { return u.fd == fd; });
    return it != _users.end() ? &(*it) : nullptr;
}

bool ServerState::removeUser(const uuid_t &uuid)
{
    return _removeByUuid(_users, uuid);
}

Team &ServerState::createTeam(const std::string &name,
                              const std::string &description, const uuid_t &creator_uuid)
{
    uuid_t generated_uuid;

    _generateUuid(generated_uuid);
    _teams.emplace_back(generated_uuid, name, description, creator_uuid);
    const std::string team_uuid = uuidToString(_teams.back().uuid);
    const std::string creator = uuidToString(creator_uuid);
    server_event_team_created(team_uuid.c_str(), _teams.back().name.c_str(), creator.c_str());
    return _teams.back();
}

Team *ServerState::findTeamByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_teams.begin(), _teams.end(),
                           [&uuid](const Team &t)
                           { return uuid_compare(t.uuid, uuid) == 0; });
    return it != _teams.end() ? &(*it) : nullptr;
}

Team *ServerState::findTeamByName(const std::string &name)
{
    auto it = std::find_if(_teams.begin(), _teams.end(),
                           [&name](const Team &t)
                           { return t.name == name; });
    return it != _teams.end() ? &(*it) : nullptr;
}

bool ServerState::removeTeam(const uuid_t &uuid)
{
    return _removeByUuid(_teams, uuid);
}

Subscription &ServerState::createSubscription(const uuid_t &user_uuid,
                                              const uuid_t &team_uuid)
{
    if (subscriptionExists(user_uuid, team_uuid))
        throw std::runtime_error("Subscription already exists");
    _subscriptions.emplace_back(user_uuid, team_uuid);
    return _subscriptions.back();
}

bool ServerState::subscriptionExists(const uuid_t &user_uuid,
                                     const uuid_t &team_uuid) const
{
    return std::any_of(_subscriptions.begin(), _subscriptions.end(),
                       [&](const Subscription &s)
                       {
                           return uuid_compare(s.user_uuid, user_uuid) == 0 &&
                                  uuid_compare(s.team_uuid, team_uuid) == 0;
                       });
}

bool ServerState::removeSubscription(const uuid_t &user_uuid,
                                     const uuid_t &team_uuid)
{
    auto it = std::find_if(_subscriptions.begin(), _subscriptions.end(),
                           [&](const Subscription &s)
                           {
                               return uuid_compare(s.user_uuid, user_uuid) == 0 &&
                                      uuid_compare(s.team_uuid, team_uuid) == 0;
                           });
    if (it == _subscriptions.end())
        return false;
    _subscriptions.erase(it);
    const std::string team = uuidToString(team_uuid);
    const std::string user = uuidToString(user_uuid);
    server_event_user_unsubscribed(team.c_str(), user.c_str());
    return true;
}

std::vector<User *> ServerState::getTeamMembers(const uuid_t &team_uuid)
{
    std::vector<User *> members;

    for (auto &sub : _subscriptions)
    {
        if (uuid_compare(sub.team_uuid, team_uuid) != 0)
            continue;
        User *u = findUserByUuid(sub.user_uuid);
        if (u)
            members.push_back(u);
    }
    return members;
}

std::vector<Team *> ServerState::getUserTeams(const uuid_t &user_uuid)
{
    std::vector<Team *> teams;

    for (auto &sub : _subscriptions)
    {
        if (uuid_compare(sub.user_uuid, user_uuid) != 0)
            continue;
        Team *t = findTeamByUuid(sub.team_uuid);
        if (t)
            teams.push_back(t);
    }
    return teams;
}

Channel &ServerState::createChannel(const std::string &name,
                                    const std::string &description, const uuid_t &team_uuid,
                                    const uuid_t &creator_uuid)
{
    uuid_t generated_uuid;

    _generateUuid(generated_uuid);
    _channels.emplace_back(generated_uuid, name, description,
                           team_uuid, creator_uuid);
    const std::string team = uuidToString(team_uuid);
    const std::string channel = uuidToString(_channels.back().uuid);
    server_event_channel_created(team.c_str(), channel.c_str(), _channels.back().name.c_str());
    return _channels.back();
}

Channel *ServerState::findChannelByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_channels.begin(), _channels.end(),
                           [&uuid](const Channel &c)
                           { return uuid_compare(c.uuid, uuid) == 0; });
    return it != _channels.end() ? &(*it) : nullptr;
}

Channel *ServerState::findChannelByNameInTeam(const std::string &name,
                                              const uuid_t &team_uuid)
{
    auto it = std::find_if(_channels.begin(), _channels.end(),
                           [&](const Channel &c)
                           {
                               return c.name == name &&
                                      uuid_compare(c.team_uuid, team_uuid) == 0;
                           });
    return it != _channels.end() ? &(*it) : nullptr;
}

std::vector<Channel *> ServerState::getTeamChannels(const uuid_t &team_uuid)
{
    std::vector<Channel *> result;

    for (auto &c : _channels)
        if (uuid_compare(c.team_uuid, team_uuid) == 0)
            result.push_back(&c);
    return result;
}

bool ServerState::removeChannel(const uuid_t &uuid)
{
    return _removeByUuid(_channels, uuid);
}

Thread &ServerState::createThread(const std::string &title,
                                  const std::string &body, const uuid_t &channel_uuid,
                                  const uuid_t &creator_uuid)
{
    uuid_t generated_uuid;

    _generateUuid(generated_uuid);
    _threads.emplace_back(generated_uuid, title, body,
                          channel_uuid, creator_uuid);
    const std::string channel = uuidToString(channel_uuid);
    const std::string thread = uuidToString(_threads.back().uuid);
    const std::string creator = uuidToString(creator_uuid);
    server_event_thread_created(channel.c_str(), thread.c_str(), creator.c_str(),
                                _threads.back().title.c_str(), _threads.back().body.c_str());
    return _threads.back();
}

Thread *ServerState::findThreadByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_threads.begin(), _threads.end(),
                           [&uuid](const Thread &t)
                           { return uuid_compare(t.uuid, uuid) == 0; });
    return it != _threads.end() ? &(*it) : nullptr;
}

std::vector<Thread *> ServerState::getChannelThreads(
    const uuid_t &channel_uuid)
{
    std::vector<Thread *> result;

    for (auto &t : _threads)
        if (uuid_compare(t.channel_uuid, channel_uuid) == 0)
            result.push_back(&t);
    return result;
}

bool ServerState::removeThread(const uuid_t &uuid)
{
    return _removeByUuid(_threads, uuid);
}

Comment &ServerState::createComment(const std::string &body,
                                    const uuid_t &thread_uuid, const uuid_t &author_uuid)
{
    uuid_t generated_uuid;

    _generateUuid(generated_uuid);
    _comments.emplace_back(generated_uuid, body, thread_uuid, author_uuid);
    const std::string thread = uuidToString(thread_uuid);
    const std::string author = uuidToString(author_uuid);
    server_event_reply_created(thread.c_str(), author.c_str(), _comments.back().body.c_str());
    return _comments.back();
}

Comment *ServerState::findCommentByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_comments.begin(), _comments.end(),
                           [&uuid](const Comment &c)
                           { return uuid_compare(c.uuid, uuid) == 0; });
    return it != _comments.end() ? &(*it) : nullptr;
}

std::vector<Comment *> ServerState::getThreadComments(
    const uuid_t &thread_uuid)
{
    std::vector<Comment *> result;

    for (auto &c : _comments)
        if (uuid_compare(c.thread_uuid, thread_uuid) == 0)
            result.push_back(&c);
    return result;
}

bool ServerState::removeComment(const uuid_t &uuid)
{
    return _removeByUuid(_comments, uuid);
}

Message &ServerState::createMessage(const std::string &body,
                                    const uuid_t &sender_uuid, const uuid_t &receiver_uuid)
{
    uuid_t generated_uuid;

    _generateUuid(generated_uuid);
    _messages.emplace_back(generated_uuid, body, sender_uuid, receiver_uuid);
    const std::string sender = uuidToString(sender_uuid);
    const std::string receiver = uuidToString(receiver_uuid);
    server_event_private_message_sended(sender.c_str(), receiver.c_str(),
                                        _messages.back().body.c_str());
    return _messages.back();
}

std::vector<Message *> ServerState::getConversation(const uuid_t &user_a,
                                                    const uuid_t &user_b)
{
    std::vector<Message *> result;

    for (auto &m : _messages)
    {
        bool a_to_b = (uuid_compare(m.sender_uuid, user_a) == 0 &&
                       uuid_compare(m.receiver_uuid, user_b) == 0);
        bool b_to_a = (uuid_compare(m.sender_uuid, user_b) == 0 &&
                       uuid_compare(m.receiver_uuid, user_a) == 0);
        if (a_to_b || b_to_a)
            result.push_back(&m);
    }
    return result;
}

bool ServerState::removeMessage(const uuid_t &uuid)
{
    return _removeByUuid(_messages, uuid);
}

void ServerState::_writeString(std::ostream &os, const std::string &str) const
{
    size_t len = str.size();
    os.write(reinterpret_cast<const char *>(&len), sizeof(len));
    os.write(str.c_str(), len);
}

std::string ServerState::_readString(std::istream &is) const
{
    size_t len = 0;
    if (!is.read(reinterpret_cast<char *>(&len), sizeof(len))) return "";
    std::string str(len, '\0');
    if (len > 0) is.read(&str[0], len);
    return str;
}

int ServerState::save(const std::string &filepath) const
{
    std::ofstream os(filepath, std::ios::binary);
    if (!os)
        return -1;

    _saveUsers(os);
    _saveTeams(os);
    _saveSubscriptions(os);
    _saveChannels(os);
    _saveThreads(os);
    _saveComments(os);
    _saveMessages(os);

    return 0;
}

void ServerState::_saveUsers(std::ofstream &os) const
{
    size_t count = _users.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &u : _users) {
        os.write(reinterpret_cast<const char *>(u.uuid), sizeof(uuid_t));
        _writeString(os, u.name);
    }
}

void ServerState::_saveTeams(std::ofstream &os) const
{
    size_t count = _teams.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &t : _teams) {
        os.write(reinterpret_cast<const char *>(t.uuid), sizeof(uuid_t));
        _writeString(os, t.name);
        _writeString(os, t.description);
        os.write(reinterpret_cast<const char *>(t.creator_uuid), sizeof(uuid_t));
    }
}

void ServerState::_saveSubscriptions(std::ofstream &os) const
{
    size_t count = _subscriptions.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &s : _subscriptions) {
        os.write(reinterpret_cast<const char *>(s.user_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(s.team_uuid), sizeof(uuid_t));
    }
}

void ServerState::_saveChannels(std::ofstream &os) const
{
    size_t count = _channels.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &c : _channels) {
        os.write(reinterpret_cast<const char *>(c.uuid), sizeof(uuid_t));
        _writeString(os, c.name);
        _writeString(os, c.description);
        os.write(reinterpret_cast<const char *>(c.team_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(c.creator_uuid), sizeof(uuid_t));
    }
}

void ServerState::_saveThreads(std::ofstream &os) const
{
    size_t count = _threads.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &t : _threads) {
        os.write(reinterpret_cast<const char *>(t.uuid), sizeof(uuid_t));
        _writeString(os, t.title);
        _writeString(os, t.body);
        os.write(reinterpret_cast<const char *>(t.channel_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(t.creator_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(&t.timestamp), sizeof(time_t));
    }
}

void ServerState::_saveComments(std::ofstream &os) const
{
    size_t count = _comments.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &c : _comments) {
        os.write(reinterpret_cast<const char *>(c.uuid), sizeof(uuid_t));
        _writeString(os, c.body);
        os.write(reinterpret_cast<const char *>(c.thread_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(c.author_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(&c.timestamp), sizeof(time_t));
    }
}

void ServerState::_saveMessages(std::ofstream &os) const
{
    size_t count = _messages.size();
    os.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (const auto &m : _messages) {
        os.write(reinterpret_cast<const char *>(m.uuid), sizeof(uuid_t));
        _writeString(os, m.body);
        os.write(reinterpret_cast<const char *>(m.sender_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(m.receiver_uuid), sizeof(uuid_t));
        os.write(reinterpret_cast<const char *>(&m.timestamp), sizeof(time_t));
    }
}

int ServerState::load(const std::string &filepath)
{
    std::ifstream is(filepath, std::ios::binary);
    if (!is)
        return -1;

    _loadUsers(is);
    _loadTeams(is);
    _loadSubscriptions(is);
    _loadChannels(is);
    _loadThreads(is);
    _loadComments(is);
    _loadMessages(is);

    return 0;
}

int ServerState::_loadUsers(std::ifstream &is)
{
    size_t count;
    _users.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            User u;
            is.read(reinterpret_cast<char *>(u.uuid), sizeof(uuid_t));
            u.name = _readString(is);
            u.fd = -1;
            u.is_logged_in = false;
            _users.push_back(u);
            server_event_user_loaded(uuidToString(u.uuid).c_str(), u.name.c_str());
        }
    }
    return 0;
}

int ServerState::_loadTeams(std::ifstream &is)
{
    size_t count;
    _teams.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            Team t;
            is.read(reinterpret_cast<char *>(t.uuid), sizeof(uuid_t));
            t.name = _readString(is);
            t.description = _readString(is);
            is.read(reinterpret_cast<char *>(t.creator_uuid), sizeof(uuid_t));
            _teams.push_back(t);
        }
    }
    return 0;
}

int ServerState::_loadSubscriptions(std::ifstream &is)
{
    size_t count;
    _subscriptions.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            Subscription s;
            is.read(reinterpret_cast<char *>(s.user_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(s.team_uuid), sizeof(uuid_t));
            _subscriptions.push_back(s);
        }
    }
    return 0;
}

int ServerState::_loadChannels(std::ifstream &is)
{
    size_t count;
    _channels.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            Channel c;
            is.read(reinterpret_cast<char *>(c.uuid), sizeof(uuid_t));
            c.name = _readString(is);
            c.description = _readString(is);
            is.read(reinterpret_cast<char *>(c.team_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(c.creator_uuid), sizeof(uuid_t));
            _channels.push_back(c);
        }
    }
    return 0;
}

int ServerState::_loadThreads(std::ifstream &is)
{
    size_t count;
    _threads.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            Thread t;
            is.read(reinterpret_cast<char *>(t.uuid), sizeof(uuid_t));
            t.title = _readString(is);
            t.body = _readString(is);
            is.read(reinterpret_cast<char *>(t.channel_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(t.creator_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(&t.timestamp), sizeof(time_t));
            _threads.push_back(t);
        }
    }
    return 0;
}

int ServerState::_loadComments(std::ifstream &is)
{
    size_t count;
    _comments.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            Comment c;
            is.read(reinterpret_cast<char *>(c.uuid), sizeof(uuid_t));
            c.body = _readString(is);
            is.read(reinterpret_cast<char *>(c.thread_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(c.author_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(&c.timestamp), sizeof(time_t));
            _comments.push_back(c);
        }
    }
    return 0;
}

int ServerState::_loadMessages(std::ifstream &is)
{
    size_t count;
    _messages.clear();
    if (is.read(reinterpret_cast<char *>(&count), sizeof(count))) {
        for (size_t i = 0; i < count; ++i) {
            Message m;
            is.read(reinterpret_cast<char *>(m.uuid), sizeof(uuid_t));
            m.body = _readString(is);
            is.read(reinterpret_cast<char *>(m.sender_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(m.receiver_uuid), sizeof(uuid_t));
            is.read(reinterpret_cast<char *>(&m.timestamp), sizeof(time_t));
            _messages.push_back(m);
        }
    }
    return 0;
}
