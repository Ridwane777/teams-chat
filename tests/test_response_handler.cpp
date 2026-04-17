#include "../include/client/ResponseHandler.hpp"
#include "logging_spy.hpp"
#include "test_helpers.hpp"

TEST_CASE(response_handler_error_dispatch)
{
    reset_logging_spy();
    ResponseHandler::execute("ERR", {{"code", "401"}});
    ResponseHandler::execute("ERR", {{"code", "404"}, {"reason", "team_not_found"}, {"uuid", "t"}});
    ResponseHandler::execute("ERR", {{"code", "404"}, {"reason", "channel_not_found"}, {"uuid", "c"}});
    ResponseHandler::execute("ERR", {{"code", "404"}, {"reason", "thread_not_found"}, {"uuid", "h"}});
    ResponseHandler::execute("ERR", {{"code", "404"}, {"reason", "user_not_found"}, {"uuid", "u"}});
    ResponseHandler::execute("ERR", {{"code", "409"}});
    ASSERT_EQ(logging_call_count("client_error_unauthorized"), 1);
    ASSERT_EQ(logging_call_count("client_error_unknown_team"), 1);
    ASSERT_EQ(logging_call_count("client_error_unknown_channel"), 1);
    ASSERT_EQ(logging_call_count("client_error_unknown_thread"), 1);
    ASSERT_EQ(logging_call_count("client_error_unknown_user"), 1);
    ASSERT_EQ(logging_call_count("client_error_already_exist"), 1);
}

TEST_CASE(response_handler_event_dispatch)
{
    reset_logging_spy();
    ResponseHandler::execute("EVT", {{"event", "USER_ONLINE"}, {"uuid", "u"}, {"name", "n"}});
    ResponseHandler::execute("EVT", {{"event", "USER_OFFLINE"}, {"uuid", "u"}, {"name", "n"}});
    ResponseHandler::execute("EVT", {{"event", "MSG_RECEIVED"}, {"from", "u"}, {"body", "b"}});
    ResponseHandler::execute("EVT", {{"event", "TEAM_CREATED"}, {"uuid", "u"}, {"name", "n"}, {"description", "d"}});
    ResponseHandler::execute("EVT", {{"event", "CHANNEL_CREATED"}, {"uuid", "u"}, {"name", "n"}, {"description", "d"}});
    ResponseHandler::execute("EVT", {{"event", "THREAD_CREATED"}, {"uuid", "u"}, {"creator", "c"}, {"timestamp", "1"}, {"title", "t"}, {"body", "b"}});
    ResponseHandler::execute("EVT", {{"event", "COMMENT_CREATED"}, {"team", "t"}, {"thread", "h"}, {"author", "u"}, {"body", "b"}});
    ASSERT_EQ(logging_call_count("client_event_logged_in"), 1);
    ASSERT_EQ(logging_call_count("client_event_logged_out"), 1);
    ASSERT_EQ(logging_call_count("client_event_private_message_received"), 1);
    ASSERT_EQ(logging_call_count("client_event_team_created"), 1);
    ASSERT_EQ(logging_call_count("client_event_channel_created"), 1);
    ASSERT_EQ(logging_call_count("client_event_thread_created"), 1);
    ASSERT_EQ(logging_call_count("client_event_thread_reply_received"), 1);
}

TEST_CASE(response_handler_ok_dispatch_all_pending_commands)
{
    reset_logging_spy();

    ResponseHandler::setPendingCommand("/login");
    ResponseHandler::execute("OK", {{"logged_in", "1"}, {"uuid", "u1"}, {"name", "alice"}});
    ResponseHandler::setPendingCommand("/users");
    ResponseHandler::execute("OK", {{"logged_in", "1"}, {"uuid", "u2"}, {"name", "bob"}});
    ResponseHandler::setPendingCommand("/user");
    ResponseHandler::execute("OK", {{"logged_in", "0"}, {"uuid", "u3"}, {"name", "charlie"}});
    ResponseHandler::setPendingCommand("/messages");
    ResponseHandler::execute("OK", {{"from", "u2"}, {"timestamp", "1"}, {"body", "hello"}});
    ResponseHandler::setPendingCommand("/subscribe");
    ResponseHandler::execute("OK", {{"team", "t1"}});
    ResponseHandler::setPendingCommand("/unsubscribe");
    ResponseHandler::execute("OK", {{"team", "t1"}});
    ResponseHandler::setPendingCommand("/subscribed_teams");
    ResponseHandler::execute("OK", {{"uuid", "t1"}, {"name", "team"}, {"description", "d"}});
    ResponseHandler::setPendingCommand("/subscribed_users");
    ResponseHandler::execute("OK", {{"uuid", "u1"}, {"name", "alice"}, {"logged_in", "1"}});
    ResponseHandler::setPendingCommand("/create_team");
    ResponseHandler::execute("OK", {{"uuid", "t2"}, {"name", "team2"}, {"description", "d2"}, {"creator", "u1"}});
    ResponseHandler::setPendingCommand("/create_channel");
    ResponseHandler::execute("OK", {{"uuid", "c1"}, {"name", "chan"}, {"description", "d"}, {"team", "t2"}});
    ResponseHandler::setPendingCommand("/create_thread");
    ResponseHandler::execute("OK", {{"uuid", "th1"}, {"creator", "u1"}, {"timestamp", "2"}, {"title", "ti"}, {"body", "bo"}});
    ResponseHandler::setPendingCommand("/create_reply");
    ResponseHandler::execute("OK", {{"thread", "th1"}, {"author", "u2"}, {"timestamp", "3"}, {"body", "rep"}});
    ResponseHandler::setPendingCommand("/list_teams");
    ResponseHandler::execute("OK", {{"uuid", "t1"}, {"name", "tn"}, {"description", "td"}});
    ResponseHandler::setPendingCommand("/list_channels");
    ResponseHandler::execute("OK", {{"uuid", "c1"}, {"name", "cn"}, {"description", "cd"}, {"team", "t1"}});
    ResponseHandler::setPendingCommand("/list_threads");
    ResponseHandler::execute("OK", {{"uuid", "th1"}, {"creator", "u1"}, {"timestamp", "4"}, {"title", "tt"}, {"body", "tb"}});
    ResponseHandler::setPendingCommand("/list_replies");
    ResponseHandler::execute("OK", {{"thread", "th1"}, {"author", "u1"}, {"timestamp", "5"}, {"body", "rb"}});
    ResponseHandler::setPendingCommand("/info_user");
    ResponseHandler::execute("OK", {{"uuid", "u1"}, {"name", "alice"}, {"logged_in", "1"}});
    ResponseHandler::setPendingCommand("/info_team");
    ResponseHandler::execute("OK", {{"uuid", "t1"}, {"name", "t"}, {"description", "d"}, {"creator", "u1"}});
    ResponseHandler::setPendingCommand("/info_channel");
    ResponseHandler::execute("OK", {{"uuid", "c1"}, {"name", "c"}, {"description", "d"}, {"team", "t1"}});
    ResponseHandler::setPendingCommand("/info_thread");
    ResponseHandler::execute("OK", {{"uuid", "th1"}, {"creator", "u1"}, {"timestamp", "6"}, {"title", "t"}, {"body", "b"}});

    ASSERT_TRUE(logging_call_count("client_event_logged_in") >= 1);
    ASSERT_TRUE(logging_call_count("client_print_users") >= 2);
    ASSERT_TRUE(logging_call_count("client_print_user") >= 2);
    ASSERT_EQ(logging_call_count("client_private_message_print_messages"), 1);
    ASSERT_EQ(logging_call_count("client_print_subscribed"), 1);
    ASSERT_EQ(logging_call_count("client_print_unsubscribed"), 1);
    ASSERT_TRUE(logging_call_count("client_print_teams") >= 2);
    ASSERT_EQ(logging_call_count("client_print_team_created"), 1);
    ASSERT_EQ(logging_call_count("client_print_channel_created"), 1);
    ASSERT_EQ(logging_call_count("client_print_thread_created"), 1);
    ASSERT_EQ(logging_call_count("client_print_reply_created"), 1);
    ASSERT_EQ(logging_call_count("client_team_print_channels"), 1);
    ASSERT_EQ(logging_call_count("client_channel_print_threads"), 1);
    ASSERT_EQ(logging_call_count("client_thread_print_replies"), 1);
    ASSERT_EQ(logging_call_count("client_print_team"), 1);
    ASSERT_EQ(logging_call_count("client_print_channel"), 1);
    ASSERT_EQ(logging_call_count("client_print_thread"), 1);
}

TEST_CASE(response_handler_ok_ignores_count_and_unknown)
{
    reset_logging_spy();
    ResponseHandler::setPendingCommand("/users");
    ResponseHandler::execute("OK", {{"count", "2"}});
    ResponseHandler::execute("XYZ", {{"a", "b"}});
    ASSERT_EQ(logging_call_count("client_print_users"), 0);
}
