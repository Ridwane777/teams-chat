#include "../include/Entities.hpp"

User::User(const uuid_t &uuid, std::string name, int fd)
    : name(std::move(name)), is_logged_in(true), fd(fd)
{
    uuid_copy(this->uuid, uuid);
}

Team::Team(const uuid_t &uuid, std::string name, std::string description,
           const uuid_t &creator_uuid)
    : name(std::move(name)), description(std::move(description))
{
    uuid_copy(this->uuid, uuid);
    uuid_copy(this->creator_uuid, creator_uuid);
}

Subscription::Subscription(const uuid_t &user_uuid, const uuid_t &team_uuid)
{
    uuid_copy(this->user_uuid, user_uuid);
    uuid_copy(this->team_uuid, team_uuid);
}

Channel::Channel(const uuid_t &uuid, std::string name, std::string description,
                 const uuid_t &team_uuid, const uuid_t &creator_uuid)
    : name(std::move(name)), description(std::move(description))
{
    uuid_copy(this->uuid, uuid);
    uuid_copy(this->team_uuid, team_uuid);
    uuid_copy(this->creator_uuid, creator_uuid);
}

Thread::Thread(const uuid_t &uuid, std::string title, std::string body,
               const uuid_t &channel_uuid, const uuid_t &creator_uuid)
    : title(std::move(title)), body(std::move(body)), timestamp(std::time(nullptr))
{
    uuid_copy(this->uuid, uuid);
    uuid_copy(this->channel_uuid, channel_uuid);
    uuid_copy(this->creator_uuid, creator_uuid);
}

Comment::Comment(const uuid_t &uuid, std::string body, const uuid_t &thread_uuid,
                 const uuid_t &author_uuid)
    : body(std::move(body)), timestamp(std::time(nullptr))
{
    uuid_copy(this->uuid, uuid);
    uuid_copy(this->thread_uuid, thread_uuid);
    uuid_copy(this->author_uuid, author_uuid);
}

Message::Message(const uuid_t &uuid, std::string body, const uuid_t &sender_uuid,
                 const uuid_t &receiver_uuid)
    : body(std::move(body)), timestamp(std::time(nullptr))
{
    uuid_copy(this->uuid, uuid);
    uuid_copy(this->sender_uuid, sender_uuid);
    uuid_copy(this->receiver_uuid, receiver_uuid);
}
