#include "logging_spy.hpp"
#include "../include/client/client.hpp"
#include <unordered_map>
#include <ctime>
#include <string>

static std::unordered_map<std::string, int> g_calls;

void reset_logging_spy()
{
    g_calls.clear();
}

int logging_call_count(const std::string &name)
{
    auto it = g_calls.find(name);
    return it == g_calls.end() ? 0 : it->second;
}

static int hit(const std::string &name)
{
    g_calls[name] += 1;
    return 0;
}

extern "C" {
int client_event_logged_in(const char *, const char *) { return hit(__func__); }
int client_event_logged_out(const char *, const char *) { return hit(__func__); }
int client_event_private_message_received(const char *, const char *) { return hit(__func__); }
int client_event_thread_reply_received(const char *, const char *, const char *, const char *) { return hit(__func__); }
int client_event_team_created(const char *, const char *, const char *) { return hit(__func__); }
int client_event_channel_created(const char *, const char *, const char *) { return hit(__func__); }
int client_event_thread_created(const char *, const char *, time_t, const char *, const char *) { return hit(__func__); }
int client_print_users(const char *, const char *, int) { return hit(__func__); }
int client_print_teams(const char *, const char *, const char *) { return hit(__func__); }
int client_team_print_channels(const char *, const char *, const char *) { return hit(__func__); }
int client_channel_print_threads(const char *, const char *, time_t, const char *, const char *) { return hit(__func__); }
int client_thread_print_replies(const char *, const char *, time_t, const char *) { return hit(__func__); }
int client_private_message_print_messages(const char *, time_t, const char *) { return hit(__func__); }
int client_error_unknown_team(const char *) { return hit(__func__); }
int client_error_unknown_channel(const char *) { return hit(__func__); }
int client_error_unknown_thread(const char *) { return hit(__func__); }
int client_error_unknown_user(const char *) { return hit(__func__); }
int client_error_unauthorized(void) { return hit(__func__); }
int client_error_already_exist(void) { return hit(__func__); }
int client_print_user(const char *, const char *, int) { return hit(__func__); }
int client_print_team(const char *, const char *, const char *) { return hit(__func__); }
int client_print_channel(const char *, const char *, const char *) { return hit(__func__); }
int client_print_thread(const char *, const char *, time_t, const char *, const char *) { return hit(__func__); }
int client_print_team_created(const char *, const char *, const char *) { return hit(__func__); }
int client_print_channel_created(const char *, const char *, const char *) { return hit(__func__); }
int client_print_thread_created(const char *, const char *, time_t, const char *, const char *) { return hit(__func__); }
int client_print_reply_created(const char *, const char *, time_t, const char *) { return hit(__func__); }
int client_print_subscribed(const char *, const char *) { return hit(__func__); }
int client_print_unsubscribed(const char *, const char *) { return hit(__func__); }

int server_event_team_created(const char *, const char *, const char *) { return hit(__func__); }
int server_event_channel_created(const char *, const char *, const char *) { return hit(__func__); }
int server_event_thread_created(const char *, const char *, const char *, const char *, const char *) { return hit(__func__); }
int server_event_reply_created(const char *, const char *, const char *) { return hit(__func__); }
int server_event_user_subscribed(const char *, const char *) { return hit(__func__); }
int server_event_user_unsubscribed(const char *, const char *) { return hit(__func__); }
int server_event_user_created(const char *, const char *) { return hit(__func__); }
int server_event_user_loaded(const char *, const char *) { return hit(__func__); }
int server_event_user_logged_in(const char *) { return hit(__func__); }
int server_event_user_logged_out(const char *) { return hit(__func__); }
int server_event_private_message_sended(const char *, const char *, const char *) { return hit(__func__); }
}

std::string Client::url_encode(const std::string &str)
{
    return str;
}
