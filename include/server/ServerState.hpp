#ifndef SERVERSTATE_HPP
#define SERVERSTATE_HPP

#include "../Entities.hpp"

class ServerState
{
public:
    ServerState() = default;
    ~ServerState() = default;

    ServerState(const ServerState &) = delete;
    ServerState &operator=(const ServerState &) = delete;
    ServerState(ServerState &&) = default;
    ServerState &operator=(ServerState &&) = default;

    User &createUser(const std::string &name, int fd);
    User *findUserByUuid(const uuid_t &uuid);
    User *findUserByName(const std::string &name);
    User *findUserByFd(int fd);
    bool removeUser(const uuid_t &uuid);

    Team &createTeam(const std::string &name,
                     const std::string &description,
                     const uuid_t &creator_uuid);
    Team *findTeamByUuid(const uuid_t &uuid);
    Team *findTeamByName(const std::string &name);
    bool removeTeam(const uuid_t &uuid);

    Subscription &createSubscription(const uuid_t &user_uuid,
                                     const uuid_t &team_uuid);
    bool subscriptionExists(const uuid_t &user_uuid,
                            const uuid_t &team_uuid) const;
    bool removeSubscription(const uuid_t &user_uuid,
                            const uuid_t &team_uuid);

    std::vector<User *> getTeamMembers(const uuid_t &team_uuid);
    std::vector<Team *> getUserTeams(const uuid_t &user_uuid);

    Channel &createChannel(const std::string &name,
                           const std::string &description,
                           const uuid_t &team_uuid,
                           const uuid_t &creator_uuid);
    Channel *findChannelByUuid(const uuid_t &uuid);
    Channel *findChannelByNameInTeam(const std::string &name,
                                     const uuid_t &team_uuid);
    std::vector<Channel *> getTeamChannels(const uuid_t &team_uuid);
    bool removeChannel(const uuid_t &uuid);

    Thread &createThread(const std::string &title,
                         const std::string &body,
                         const uuid_t &channel_uuid,
                         const uuid_t &creator_uuid);
    Thread *findThreadByUuid(const uuid_t &uuid);
    std::vector<Thread *> getChannelThreads(const uuid_t &channel_uuid);
    bool removeThread(const uuid_t &uuid);

    Comment &createComment(const std::string &body,
                           const uuid_t &thread_uuid,
                           const uuid_t &author_uuid);
    Comment *findCommentByUuid(const uuid_t &uuid);
    std::vector<Comment *> getThreadComments(const uuid_t &thread_uuid);
    bool removeComment(const uuid_t &uuid);

    Message &createMessage(const std::string &body,
                           const uuid_t &sender_uuid,
                           const uuid_t &receiver_uuid);
    std::vector<Message *> getConversation(const uuid_t &user_a,
                                           const uuid_t &user_b);
    bool removeMessage(const uuid_t &uuid);

    int save(const std::string &filepath) const;
    int load(const std::string &filepath);

    std::vector<User> &users() { return _users; }
    std::vector<Team> &teams() { return _teams; }
    std::vector<Subscription> &subscriptions() { return _subscriptions; }
    std::vector<Channel> &channels() { return _channels; }
    std::vector<Thread> &threads() { return _threads; }
    std::vector<Comment> &comments() { return _comments; }
    std::vector<Message> &messages() { return _messages; }

private:
    std::vector<User> _users;
    std::vector<Team> _teams;
    std::vector<Subscription> _subscriptions;
    std::vector<Channel> _channels;
    std::vector<Thread> _threads;
    std::vector<Comment> _comments;
    std::vector<Message> _messages;

    static uuid_t _generateUuid();

    template <typename T>
    bool _removeByUuid(std::vector<T> &vec, const uuid_t &uuid);
};

#endif