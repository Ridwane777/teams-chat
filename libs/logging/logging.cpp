// SPDX-License-Identifier: MIT
// Clean-room implementation of the teams-chat logging API.
// Client functions print to stdout (errors to stderr); server functions log to stderr.

#include "logging_client.h"
#include "logging_server.h"

#include <cstdio>
#include <ctime>

namespace {

const char *s(const char *v) { return v ? v : ""; }

// Formats a timestamp as "YYYY-MM-DD HH:MM:SS" (local time).
const char *fmt_time(time_t t, char *buf, size_t len)
{
    struct tm tmv {};
    if (localtime_r(&t, &tmv) == nullptr || std::strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tmv) == 0)
        std::snprintf(buf, len, "%lld", static_cast<long long>(t));
    return buf;
}

const char *status(int st) { return st ? "online" : "offline"; }

} // namespace

#define CLIENT_OUT(...) do { std::printf(__VA_ARGS__); std::fflush(stdout); return 0; } while (0)
#define CLIENT_ERR(...) do { std::fprintf(stderr, "Error: " __VA_ARGS__); return 0; } while (0)
#define SERVER_LOG(...) do { std::fprintf(stderr, "[server] " __VA_ARGS__); return 0; } while (0)

extern "C" {

int client_event_logged_in(char const *u, const char *n) { CLIENT_OUT("User \"%s\" (%s) logged in\n", s(n), s(u)); }
int client_event_logged_out(char const *u, const char *n) { CLIENT_OUT("User \"%s\" (%s) logged out\n", s(n), s(u)); }
int client_event_private_message_received(char const *u, char const *b) { CLIENT_OUT("New message from %s: %s\n", s(u), s(b)); }
int client_event_thread_reply_received(char const *t, char const *th, char const *u, char const *b)
{ CLIENT_OUT("New reply in thread %s (team %s) from %s: %s\n", s(th), s(t), s(u), s(b)); }
int client_event_team_created(char const *id, char const *n, char const *d) { CLIENT_OUT("New team \"%s\" (%s): %s\n", s(n), s(id), s(d)); }
int client_event_channel_created(char const *id, char const *n, char const *d) { CLIENT_OUT("New channel \"%s\" (%s): %s\n", s(n), s(id), s(d)); }
int client_event_thread_created(char const *id, char const *u, time_t ts, char const *ti, char const *b)
{ char tb[32]; CLIENT_OUT("New thread \"%s\" (%s) by %s at %s: %s\n", s(ti), s(id), s(u), fmt_time(ts, tb, sizeof tb), s(b)); }

int client_print_users(char const *id, char const *n, int st) { CLIENT_OUT("- %s (%s) [%s]\n", s(n), s(id), status(st)); }
int client_print_teams(char const *id, char const *n, char const *d) { CLIENT_OUT("- Team \"%s\" (%s): %s\n", s(n), s(id), s(d)); }
int client_team_print_channels(char const *id, char const *n, char const *d) { CLIENT_OUT("- Channel \"%s\" (%s): %s\n", s(n), s(id), s(d)); }
int client_channel_print_threads(char const *id, char const *u, time_t ts, char const *ti, char const *b)
{ char tb[32]; CLIENT_OUT("- Thread \"%s\" (%s) by %s at %s: %s\n", s(ti), s(id), s(u), fmt_time(ts, tb, sizeof tb), s(b)); }
int client_thread_print_replies(char const *th, char const *u, time_t ts, char const *b)
{ char tb[32]; CLIENT_OUT("- Reply in %s by %s at %s: %s\n", s(th), s(u), fmt_time(ts, tb, sizeof tb), s(b)); }
int client_private_message_print_messages(char const *u, time_t ts, char const *b)
{ char tb[32]; CLIENT_OUT("- [%s] %s: %s\n", fmt_time(ts, tb, sizeof tb), s(u), s(b)); }

int client_error_unknown_team(char const *id) { CLIENT_ERR("unknown team %s\n", s(id)); }
int client_error_unknown_channel(char const *id) { CLIENT_ERR("unknown channel %s\n", s(id)); }
int client_error_unknown_thread(char const *id) { CLIENT_ERR("unknown thread %s\n", s(id)); }
int client_error_unknown_user(char const *id) { CLIENT_ERR("unknown user %s\n", s(id)); }
int client_error_unauthorized(void) { CLIENT_ERR("unauthorized (log in or subscribe first)\n"); }
int client_error_already_exist(void) { CLIENT_ERR("resource already exists\n"); }

int client_print_user(char const *id, char const *n, int st) { CLIENT_OUT("User \"%s\" (%s) [%s]\n", s(n), s(id), status(st)); }
int client_print_team(char const *id, char const *n, char const *d) { CLIENT_OUT("Team \"%s\" (%s): %s\n", s(n), s(id), s(d)); }
int client_print_channel(char const *id, char const *n, char const *d) { CLIENT_OUT("Channel \"%s\" (%s): %s\n", s(n), s(id), s(d)); }
int client_print_thread(char const *id, char const *u, time_t ts, char const *ti, char const *b)
{ char tb[32]; CLIENT_OUT("Thread \"%s\" (%s) by %s at %s: %s\n", s(ti), s(id), s(u), fmt_time(ts, tb, sizeof tb), s(b)); }
int client_print_team_created(char const *id, char const *n, char const *d) { CLIENT_OUT("Team \"%s\" (%s) created: %s\n", s(n), s(id), s(d)); }
int client_print_channel_created(char const *id, char const *n, char const *d) { CLIENT_OUT("Channel \"%s\" (%s) created: %s\n", s(n), s(id), s(d)); }
int client_print_thread_created(char const *id, char const *u, time_t ts, char const *ti, char const *b)
{ char tb[32]; CLIENT_OUT("Thread \"%s\" (%s) created by %s at %s: %s\n", s(ti), s(id), s(u), fmt_time(ts, tb, sizeof tb), s(b)); }
int client_print_reply_created(char const *th, char const *u, time_t ts, char const *b)
{ char tb[32]; CLIENT_OUT("Reply posted in %s by %s at %s: %s\n", s(th), s(u), fmt_time(ts, tb, sizeof tb), s(b)); }
int client_print_subscribed(char const *u, char const *t) { CLIENT_OUT("User %s subscribed to team %s\n", s(u), s(t)); }
int client_print_unsubscribed(char const *u, char const *t) { CLIENT_OUT("User %s unsubscribed from team %s\n", s(u), s(t)); }

int server_event_team_created(char const *t, char const *n, char const *u) { SERVER_LOG("team created %s \"%s\" by %s\n", s(t), s(n), s(u)); }
int server_event_channel_created(char const *t, char const *c, char const *n) { SERVER_LOG("channel created %s \"%s\" in team %s\n", s(c), s(n), s(t)); }
int server_event_thread_created(char const *c, char const *th, char const *u, char const *ti, char const *b)
{ SERVER_LOG("thread created %s \"%s\" in channel %s by %s: %s\n", s(th), s(ti), s(c), s(u), s(b)); }
int server_event_reply_created(char const *th, char const *u, char const *b) { SERVER_LOG("reply created in thread %s by %s: %s\n", s(th), s(u), s(b)); }
int server_event_user_subscribed(char const *t, char const *u) { SERVER_LOG("user %s subscribed to team %s\n", s(u), s(t)); }
int server_event_user_unsubscribed(char const *t, char const *u) { SERVER_LOG("user %s unsubscribed from team %s\n", s(u), s(t)); }
int server_event_user_created(char const *u, char const *n) { SERVER_LOG("user created %s \"%s\"\n", s(u), s(n)); }
int server_event_user_loaded(char const *u, char const *n) { SERVER_LOG("user loaded %s \"%s\"\n", s(u), s(n)); }
int server_event_user_logged_in(char const *u) { SERVER_LOG("user logged in %s\n", s(u)); }
int server_event_user_logged_out(char const *u) { SERVER_LOG("user logged out %s\n", s(u)); }
int server_event_private_message_sended(char const *f, char const *t, char const *b) { SERVER_LOG("private message %s -> %s: %s\n", s(f), s(t), s(b)); }

} // extern "C"
