#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <map>

#undef MAX
#define MAX 4096
#define PORT 8080
#define SA struct sockaddr

class Client
{
private:
    int sockfd;
    struct sockaddr_in servaddr;
    bool _is_running;
    std::string _readBuffer;

public:
    Client();
    void create_socket();
    void connect_to_server(const std::string &ip, int port);
    void send_command(const std::string &command);
    void receive_response();
    void run();

    static std::string url_decode(const std::string &str);
    static std::string url_encode(const std::string &str);

private:
    void handle_response(const std::string &response);
    std::map<std::string, std::string> parse_payload(const std::string &payload);
};

#endif