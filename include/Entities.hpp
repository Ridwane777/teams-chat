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
    User(const uuid_t &uuid, std::string name, int fd);
};

struct Team
{
    uuid_t uuid;
    std::string name;
    std::string description;
    uuid_t creator_uuid;

    Team() = default;
        Team(const uuid_t &uuid, std::string name, std::string description,
            const uuid_t &creator_uuid);
};

struct Subscription
{
    uuid_t user_uuid;
    uuid_t team_uuid;

    Subscription() = default;
    Subscription(const uuid_t &user_uuid, const uuid_t &team_uuid);
};

struct Channel
{
    uuid_t uuid;
    std::string name;
    std::string description;
    uuid_t team_uuid;
    uuid_t creator_uuid;

    Channel() = default;
    Channel(const uuid_t &uuid, std::string name, std::string description,
            const uuid_t &team_uuid, const uuid_t &creator_uuid);
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
    Thread(const uuid_t &uuid, std::string title, std::string body,
           const uuid_t &channel_uuid, const uuid_t &creator_uuid);
};

struct Comment
{
    uuid_t uuid;
    std::string body;
    uuid_t thread_uuid;
    uuid_t author_uuid;
    time_t timestamp = 0;

    Comment() = default;
    Comment(const uuid_t &uuid, std::string body, const uuid_t &thread_uuid,
            const uuid_t &author_uuid);
};

struct Message
{
    uuid_t uuid;
    std::string body;
    uuid_t sender_uuid;
    uuid_t receiver_uuid;
    time_t timestamp = 0;

    Message() = default;
    Message(const uuid_t &uuid, std::string body, const uuid_t &sender_uuid,
            const uuid_t &receiver_uuid);
};

#endif