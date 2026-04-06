#include "../../include/server/ServerState.hpp"
#include <uuid/uuid.h>
#include <algorithm>
#include <stdexcept>

uuid_t ServerState::_generateUuid()
{
    ::uuid_t raw;
    char buf[UUID_STR_LEN];

    uuid_generate(raw);
    uuid_unparse_lower(raw, buf);
    return std::string(buf);
}

template <typename T>
bool ServerState::_removeByUuid(std::vector<T> &vec, const uuid_t &uuid)
{
    auto it = std::find_if(vec.begin(), vec.end(),
                           [&uuid](const T &e)
                           { return e.uuid == uuid; });

    if (it == vec.end())
        return false;
    vec.erase(it);
    return true;
}

User &ServerState::createUser(const std::string &name, int fd)
{
    _users.emplace_back(_generateUuid(), name, fd);
    return _users.back();
}

User *ServerState::findUserByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_users.begin(), _users.end(),
                           [&uuid](const User &u)
                           { return u.uuid == uuid; });
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
    _teams.emplace_back(_generateUuid(), name, description, creator_uuid);
    return _teams.back();
}

Team *ServerState::findTeamByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_teams.begin(), _teams.end(),
                           [&uuid](const Team &t)
                           { return t.uuid == uuid; });
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
                           return s.user_uuid == user_uuid && s.team_uuid == team_uuid;
                       });
}

bool ServerState::removeSubscription(const uuid_t &user_uuid,
                                     const uuid_t &team_uuid)
{
    auto it = std::find_if(_subscriptions.begin(), _subscriptions.end(),
                           [&](const Subscription &s)
                           {
                               return s.user_uuid == user_uuid && s.team_uuid == team_uuid;
                           });
    if (it == _subscriptions.end())
        return false;
    _subscriptions.erase(it);
    return true;
}

std::vector<User *> ServerState::getTeamMembers(const uuid_t &team_uuid)
{
    std::vector<User *> members;

    for (auto &sub : _subscriptions)
    {
        if (sub.team_uuid != team_uuid)
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
        if (sub.user_uuid != user_uuid)
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
    _channels.emplace_back(_generateUuid(), name, description,
                           team_uuid, creator_uuid);
    return _channels.back();
}

Channel *ServerState::findChannelByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_channels.begin(), _channels.end(),
                           [&uuid](const Channel &c)
                           { return c.uuid == uuid; });
    return it != _channels.end() ? &(*it) : nullptr;
}

Channel *ServerState::findChannelByNameInTeam(const std::string &name,
                                              const uuid_t &team_uuid)
{
    auto it = std::find_if(_channels.begin(), _channels.end(),
                           [&](const Channel &c)
                           {
                               return c.name == name && c.team_uuid == team_uuid;
                           });
    return it != _channels.end() ? &(*it) : nullptr;
}

std::vector<Channel *> ServerState::getTeamChannels(const uuid_t &team_uuid)
{
    std::vector<Channel *> result;

    for (auto &c : _channels)
        if (c.team_uuid == team_uuid)
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
    _threads.emplace_back(_generateUuid(), title, body,
                          channel_uuid, creator_uuid);
    return _threads.back();
}

Thread *ServerState::findThreadByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_threads.begin(), _threads.end(),
                           [&uuid](const Thread &t)
                           { return t.uuid == uuid; });
    return it != _threads.end() ? &(*it) : nullptr;
}

std::vector<Thread *> ServerState::getChannelThreads(
    const uuid_t &channel_uuid)
{
    std::vector<Thread *> result;

    for (auto &t : _threads)
        if (t.channel_uuid == channel_uuid)
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
    _comments.emplace_back(_generateUuid(), body, thread_uuid, author_uuid);
    return _comments.back();
}

Comment *ServerState::findCommentByUuid(const uuid_t &uuid)
{
    auto it = std::find_if(_comments.begin(), _comments.end(),
                           [&uuid](const Comment &c)
                           { return c.uuid == uuid; });
    return it != _comments.end() ? &(*it) : nullptr;
}

std::vector<Comment *> ServerState::getThreadComments(
    const uuid_t &thread_uuid)
{
    std::vector<Comment *> result;

    for (auto &c : _comments)
        if (c.thread_uuid == thread_uuid)
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
    _messages.emplace_back(_generateUuid(), body, sender_uuid, receiver_uuid);
    return _messages.back();
}

std::vector<Message *> ServerState::getConversation(const uuid_t &user_a,
                                                    const uuid_t &user_b)
{
    std::vector<Message *> result;

    for (auto &m : _messages)
    {
        bool a_to_b = (m.sender_uuid == user_a && m.receiver_uuid == user_b);
        bool b_to_a = (m.sender_uuid == user_b && m.receiver_uuid == user_a);
        if (a_to_b || b_to_a)
            result.push_back(&m);
    }
    return result;
}

bool ServerState::removeMessage(const uuid_t &uuid)
{
    return _removeByUuid(_messages, uuid);
}
