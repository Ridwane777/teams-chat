#include "../include/Entities.hpp"
#include "../include/Utils.hpp"
#include "test_helpers.hpp"

TEST_CASE(entities_copy_uuid_and_fields)
{
    uuid_t u1;
    uuid_generate(u1);
    User user(u1, "alice", 12);
    ASSERT_EQ(user.name, "alice");
    ASSERT_EQ(user.fd, 12);
    ASSERT_TRUE(uuid_compare(user.uuid, u1) == 0);

    uuid_t creator;
    uuid_generate(creator);
    Team team(u1, "t", "d", creator);
    ASSERT_TRUE(uuid_compare(team.creator_uuid, creator) == 0);

    Subscription s(u1, creator);
    ASSERT_TRUE(uuid_compare(s.user_uuid, u1) == 0);

    Channel c(u1, "c", "d", creator, u1);
    ASSERT_TRUE(uuid_compare(c.team_uuid, creator) == 0);

    Thread th(u1, "title", "body", creator, u1);
    ASSERT_EQ(th.title, "title");

    Comment co(u1, "body", creator, u1);
    ASSERT_EQ(co.body, "body");

    Message m(u1, "hello", creator, u1);
    ASSERT_EQ(m.body, "hello");
}
