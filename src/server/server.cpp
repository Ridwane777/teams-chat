#include "../../include/server/server.hpp"

volatile sig_atomic_t running = 1;

void print_usage() {
    std::cout << "USAGE: ./myftp port path\n" \
                 " port is the port number on which the server socket listens\n" \
                 " path is the path to the home directory for the Anonymous user" << std::endl;
}

void signalHandler(int signum)
{
    printf("Interrupt signal (%d) received. Shutting down the server...\n", signum);
    exit(signum);
}

Server::Server()
{
}

void Server::create_socket()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "socket creation failed...\n";
        exit(0);
    }
}

void Server::bind_socket(int port)
{
    explicit_bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        std::cerr << "socket bind failed...\n";
        exit(0);
    }
}


void Server::listen_socket()
{
    if ((listen(sockfd, 100)) != 0) {
        std::cerr << "Listen failed...\n";
        exit(0);
    }
}

int Server::accept_socket()
{
    len = sizeof(cli);
    connfd = accept(sockfd, (SA *)&cli, (socklen_t *)&len);
    if (connfd < 0) {
        std::cerr << "server accept failed...\n";
        return -1;
    }
    std::cout << "<- 220 Service ready for new user." << std::endl;
    const char *welcome_msg = "220 Service ready for new user.\r\n";
    write(connfd, welcome_msg, strlen(welcome_msg));
    return connfd;
}

void Server::handle_command(int client_fd, const std::string &command)
{
    (void)client_fd;
    (void)command;
}

void Server::process_client_data(int client_fd, const char *data, ssize_t len)
{
    client_buffers[client_fd].append(data, len);
    std::string &buffer = client_buffers[client_fd];
    size_t pos;

    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string command = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);
        handle_command(client_fd, command);
    }
}

void Server::handle_client()
{
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].revents & POLLIN) {
            if (clients[i].fd == sockfd) {
                int new_fd = accept_socket();
                clients.push_back({new_fd, POLLIN, 0});
            } else {
                char buffer[MAX];
                bzero(buffer, sizeof(buffer));
                ssize_t bytes_read = read(clients[i].fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    // command_handler->remove_client_session(clients[i].fd);
                    client_buffers.erase(clients[i].fd);
                    close(clients[i].fd);
                    clients.erase(clients.begin() + i);
                    --i;
                } else {
                    process_client_data(clients[i].fd, buffer, bytes_read);
                }
            }
        }
    }
}

void Server::start(int port)
{
    signal(SIGINT, signalHandler);
    signal(SIGCHLD, SIG_IGN);

    create_socket();
    clients.push_back({sockfd, POLLIN, 0});
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind_socket(port);
    listen_socket();
    while (running) {
        int ready = poll(clients.data(), clients.size(), -1);
        if (ready < 0) {
            perror("poll");
            exit(84);
        }
        handle_client();
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 84;
    if (std::string(argv[1]) == "--help") {
        print_usage();
        return 1;
    }
    int port = atoi(argv[1]);
    Server server;
    server.start(port);
    return 0;
}
