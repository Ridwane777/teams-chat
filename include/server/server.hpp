#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <poll.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <map>
#include "CommandHandler/CommandHandler.hpp"
#undef MAX
#define MAX 4096
#define PORT 8080
#define SA struct sockaddr
namespace fs = std::filesystem;

class Server
{
private:
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    std::vector<struct pollfd> clients;
    std::map<int, std::string> client_buffers;
    std::map<int, std::string> write_buffers;
    CommandHandler _command_handler;

    static Server* instance;
    static void signalHandler(int signum);

public:
    Server();
    void create_socket();
    void bind_socket(int port);
    void listen_socket();
    int accept_socket();
    void handle_client();
    void handle_command(int client_fd, const std::string &command);
    void process_client_data(int client_fd, const char *data, ssize_t len);
    void start(int port = PORT);
};

#endif