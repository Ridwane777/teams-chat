#include "../include/Entities.hpp"

User::User(uuid_t uuid, std::string name, int fd)
    : uuid(std::move(uuid)), name(std::move(name)), is_logged_in(true), fd(fd)
{
}

Team::Team(uuid_t uuid, std::string name, std::string description,
           uuid_t creator_uuid)
    : uuid(std::move(uuid)), name(std::move(name)), description(std::move(description)), creator_uuid(std::move(creator_uuid))
{
}

Subscription::Subscription(uuid_t user_uuid, uuid_t team_uuid)
    : user_uuid(std::move(user_uuid)), team_uuid(std::move(team_uuid))
{
}

Channel::Channel(uuid_t uuid, std::string name, std::string description,
                 uuid_t team_uuid, uuid_t creator_uuid)
    : uuid(std::move(uuid)), name(std::move(name)), description(std::move(description)), team_uuid(std::move(team_uuid)), creator_uuid(std::move(creator_uuid))
{
}

Thread::Thread(uuid_t uuid, std::string title, std::string body,
               uuid_t channel_uuid, uuid_t creator_uuid)
    : uuid(std::move(uuid)), title(std::move(title)), body(std::move(body)), channel_uuid(std::move(channel_uuid)), creator_uuid(std::move(creator_uuid)), timestamp(std::time(nullptr))
{
}

Comment::Comment(uuid_t uuid, std::string body, uuid_t thread_uuid,
                 uuid_t author_uuid)
    : uuid(std::move(uuid)), body(std::move(body)), thread_uuid(std::move(thread_uuid)), author_uuid(std::move(author_uuid)), timestamp(std::time(nullptr))
{
}

Message::Message(uuid_t uuid, std::string body, uuid_t sender_uuid,
                 uuid_t receiver_uuid)
    : uuid(std::move(uuid)), body(std::move(body)), sender_uuid(std::move(sender_uuid)), receiver_uuid(std::move(receiver_uuid)), timestamp(std::time(nullptr))
{
}
