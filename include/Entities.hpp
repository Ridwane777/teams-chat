#ifndef ENTITIES_HPP
#define ENTITIES_HPP
#include "Types.hpp"

struct User
{
    uuid_t uuid;
    std::string name;
    bool is_logged_in = false;
    int fd = -1;

    User() = default;
    User(uuid_t uuid, std::string name, int fd);
};

struct Team
{
    uuid_t uuid;
    std::string name;
    std::string description;
    uuid_t creator_uuid;

    Team() = default;
    Team(uuid_t uuid, std::string name, std::string description,
         uuid_t creator_uuid);
};

struct Subscription
{
    uuid_t user_uuid;
    uuid_t team_uuid;

    Subscription() = default;
    Subscription(uuid_t user_uuid, uuid_t team_uuid);
};

struct Channel
{
    uuid_t uuid;
    std::string name;
    std::string description;
    uuid_t team_uuid;
    uuid_t creator_uuid;

    Channel() = default;
    Channel(uuid_t uuid, std::string name, std::string description,
            uuid_t team_uuid, uuid_t creator_uuid);
};

struct Thread
{
    uuid_t uuid;
    std::string title;
    std::string body;
    uuid_t channel_uuid;
    uuid_t creator_uuid;
    time_t timestamp = 0;

    Thread() = default;
    Thread(uuid_t uuid, std::string title, std::string body,
           uuid_t channel_uuid, uuid_t creator_uuid);
};

struct Comment
{
    uuid_t uuid;
    std::string body;
    uuid_t thread_uuid;
    uuid_t author_uuid;
    time_t timestamp = 0;

    Comment() = default;
    Comment(uuid_t uuid, std::string body, uuid_t thread_uuid,
            uuid_t author_uuid);
};

struct Message
{
    uuid_t uuid;
    std::string body;
    uuid_t sender_uuid;
    uuid_t receiver_uuid;
    time_t timestamp = 0;

    Message() = default;
    Message(uuid_t uuid, std::string body, uuid_t sender_uuid,
            uuid_t receiver_uuid);
};

#endif