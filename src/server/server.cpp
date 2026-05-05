#include "../../include/server/server.hpp"
#include <cerrno>

volatile sig_atomic_t running = 1;
Server* Server::instance = nullptr;

void print_usage() {
    std::cout << "USAGE: ./myteams_server port\n"
                 " port is the port number on which the server socket listens" << std::endl;
}

void Server::signalHandler(int signum)
{
    if (signum == SIGINT) {
        if (Server::instance != nullptr) {
            Server::instance->_command_handler.save_state("myteams.dat");
            Server::instance->_command_handler.handle_logout_all_clients();
            exit(0);
        }
        running = 0;
    }
}

Server::Server()
{
}

void Server::create_socket()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "socket creation failed..." << std::endl;
        exit(0);
    }
}

void Server::bind_socket(int port)
{
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0) {
        std::cout << "socket bind failed..." << std::endl;
        exit(0);
    }
}


void Server::listen_socket()
{
    if ((listen(sockfd, 100)) != 0) {
        std::cout << "Listen failed..." << std::endl;
        exit(0);
    }
}

int Server::accept_socket()
{
    len = sizeof(cli);
    connfd = accept(sockfd, (SA *)&cli, (socklen_t *)&len);
    if (connfd < 0) {
        std::cout << "server accept failed..." << std::endl;
        return -1;
    }
    return connfd;
}

void Server::handle_command(int client_fd, const std::string &command)
{
    _command_handler.handle_command(client_fd, command);
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
                    _command_handler.disconnect_client_session(clients[i].fd);
                    _command_handler.unregister_client_fd(clients[i].fd);
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
    Server::instance = this;
    
    _command_handler.load_state("myteams.dat");
    create_socket();
    clients.push_back({sockfd, POLLIN, 0});
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind_socket(port);
    listen_socket();
    signal(SIGINT, &Server::signalHandler);
    signal(SIGCHLD, SIG_IGN);
    while (running) {
        int ready = poll(clients.data(), clients.size(), -1);
        if (ready < 0) {
            if (errno == EINTR)
                continue;
            std::cout << "poll failed" << std::endl;
            exit(84);
        }
        handle_client();
    }
}

int main(int argc, char *argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--help") {
        print_usage();
        return 0;
    }
    if (argc != 2)
        return 84;
    int port = atoi(argv[1]);
    Server server;
    server.start(port);
    return 0;
}
