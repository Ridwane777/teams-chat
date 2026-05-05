/*
 * Local dev on macOS: the shipped libmyteams.so is Linux ELF and cannot link
 * with Apple ld. These stubs satisfy the linker; use the real .so on Linux.
 */
extern "C" {
#include "logging_client.h"
#include "logging_server.h"

int server_event_team_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int server_event_channel_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int server_event_thread_created(char const *a, char const *b, char const *c, char const *d, char const *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return 0;
}
int server_event_reply_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int server_event_user_subscribed(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int server_event_user_unsubscribed(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int server_event_user_created(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int server_event_user_loaded(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int server_event_user_logged_in(char const *a)
{
    (void)a;
    return 0;
}
int server_event_user_logged_out(char const *a)
{
    (void)a;
    return 0;
}
int server_event_private_message_sended(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}

int client_event_logged_in(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int client_event_logged_out(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int client_event_private_message_received(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int client_event_thread_reply_received(char const *a, char const *b, char const *c, char const *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 0;
}
int client_event_team_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_event_channel_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_event_thread_created(char const *a, char const *b, time_t t, char const *c, char const *d)
{
    (void)a;
    (void)b;
    (void)t;
    (void)c;
    (void)d;
    return 0;
}
int client_print_users(char const *a, char const *b, int s)
{
    (void)a;
    (void)b;
    (void)s;
    return 0;
}
int client_print_teams(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_team_print_channels(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_channel_print_threads(char const *a, char const *b, time_t t, char const *c, char const *d)
{
    (void)a;
    (void)b;
    (void)t;
    (void)c;
    (void)d;
    return 0;
}
int client_thread_print_replies(char const *a, char const *b, time_t t, char const *c)
{
    (void)a;
    (void)b;
    (void)t;
    (void)c;
    return 0;
}
int client_private_message_print_messages(char const *a, time_t t, char const *b)
{
    (void)a;
    (void)t;
    (void)b;
    return 0;
}
int client_error_unknown_team(char const *a)
{
    (void)a;
    return 0;
}
int client_error_unknown_channel(char const *a)
{
    (void)a;
    return 0;
}
int client_error_unknown_thread(char const *a)
{
    (void)a;
    return 0;
}
int client_error_unknown_user(char const *a)
{
    (void)a;
    return 0;
}
int client_error_unauthorized(void) { return 0; }
int client_error_already_exist(void) { return 0; }
int client_print_user(char const *a, char const *b, int s)
{
    (void)a;
    (void)b;
    (void)s;
    return 0;
}
int client_print_team(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_print_channel(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_print_thread(char const *a, char const *b, time_t t, char const *c, char const *d)
{
    (void)a;
    (void)b;
    (void)t;
    (void)c;
    (void)d;
    return 0;
}
int client_print_team_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_print_channel_created(char const *a, char const *b, char const *c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
int client_print_thread_created(char const *a, char const *b, time_t t, char const *c, char const *d)
{
    (void)a;
    (void)b;
    (void)t;
    (void)c;
    (void)d;
    return 0;
}
int client_print_reply_created(char const *a, char const *b, time_t t, char const *c)
{
    (void)a;
    (void)b;
    (void)t;
    (void)c;
    return 0;
}
int client_print_subscribed(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
int client_print_unsubscribed(char const *a, char const *b)
{
    (void)a;
    (void)b;
    return 0;
}
}
