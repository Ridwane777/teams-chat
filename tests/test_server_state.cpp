#include "../include/server/ServerState.hpp"
#include "test_helpers.hpp"
#include <cstdio>

TEST_CASE(server_state_crud_and_relations)
{
    ServerState st;
    st.createUser("alice", 1);
    st.createUser("bob", 2);
    User *u1 = st.findUserByName("alice");
    User *u2 = st.findUserByName("bob");
    ASSERT_TRUE(u1 != nullptr);
    ASSERT_TRUE(u2 != nullptr);
    ASSERT_TRUE(st.findUserByName("alice") != nullptr);
    ASSERT_TRUE(st.findUserByFd(2) != nullptr);

    Team &t = st.createTeam("team", "desc", u1->uuid);
    ASSERT_TRUE(st.findTeamByName("team") != nullptr);
    st.createSubscription(u1->uuid, t.uuid);
    ASSERT_TRUE(st.subscriptionExists(u1->uuid, t.uuid));
    ASSERT_EQ(st.getTeamMembers(t.uuid).size(), static_cast<size_t>(1));
    ASSERT_EQ(st.getUserTeams(u1->uuid).size(), static_cast<size_t>(1));
    ASSERT_TRUE(st.removeSubscription(u1->uuid, t.uuid));

    Channel &ch = st.createChannel("general", "d", t.uuid, u1->uuid);
    ASSERT_TRUE(st.findChannelByNameInTeam("general", t.uuid) != nullptr);
    ASSERT_EQ(st.getTeamChannels(t.uuid).size(), static_cast<size_t>(1));

    Thread &th = st.createThread("title", "body", ch.uuid, u1->uuid);
    ASSERT_TRUE(st.findThreadByUuid(th.uuid) != nullptr);
    ASSERT_EQ(st.getChannelThreads(ch.uuid).size(), static_cast<size_t>(1));

    Comment &co = st.createComment("reply", th.uuid, u2->uuid);
    ASSERT_TRUE(st.findCommentByUuid(co.uuid) != nullptr);
    ASSERT_EQ(st.getThreadComments(th.uuid).size(), static_cast<size_t>(1));

    Message &m = st.createMessage("hi", u1->uuid, u2->uuid);
    ASSERT_EQ(st.getConversation(u1->uuid, u2->uuid).size(), static_cast<size_t>(1));
    ASSERT_TRUE(st.removeMessage(m.uuid));
    ASSERT_TRUE(st.removeComment(co.uuid));
    ASSERT_TRUE(st.removeThread(th.uuid));
    ASSERT_TRUE(st.removeChannel(ch.uuid));
    ASSERT_TRUE(st.removeTeam(t.uuid));
    ASSERT_TRUE(st.removeUser(u1->uuid));
}

TEST_CASE(server_state_save_and_load)
{
    ServerState st;
    User &u = st.createUser("alice", 4);
    Team &t = st.createTeam("team", "desc", u.uuid);
    st.createSubscription(u.uuid, t.uuid);
    Channel &c = st.createChannel("ch", "d", t.uuid, u.uuid);
    Thread &th = st.createThread("title", "body", c.uuid, u.uuid);
    st.createComment("reply", th.uuid, u.uuid);
    st.createMessage("msg", u.uuid, u.uuid);

    std::string path = "tests_state.bin";
    ASSERT_EQ(st.save(path), 0);

    ServerState loaded;
    ASSERT_EQ(loaded.load(path), 0);
    ASSERT_EQ(loaded.users().size(), static_cast<size_t>(1));
    ASSERT_EQ(loaded.teams().size(), static_cast<size_t>(1));
    ASSERT_EQ(loaded.channels().size(), static_cast<size_t>(1));
    ASSERT_EQ(loaded.threads().size(), static_cast<size_t>(1));
    ASSERT_EQ(loaded.comments().size(), static_cast<size_t>(1));
    ASSERT_EQ(loaded.messages().size(), static_cast<size_t>(1));
    std::remove(path.c_str());
}
