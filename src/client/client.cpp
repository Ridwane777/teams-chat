#include "../../include/client/client.hpp"

Client::Client() : _is_running(true)
{
}


void Client::create_socket()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "socket creation failed...\n";
        exit(0);
    }
}

void Client::connect_to_server(const std::string &ip, int port)
{
    explicit_bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported...\n";
        exit(0);
    }
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
        std::cerr << "Connection with the server failed...\n";
        exit(0);
    }
}

void Client::send_command(const std::string &command)
{
    std::string cmd_with_newline = command + "\r\n";
    write(sockfd, cmd_with_newline.c_str(), cmd_with_newline.size());
}

void Client::receive_response()
{
    char buffer[MAX];
    ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        std::cout << buffer;
    } else if (n == 0) {
        std::cout << "Server closed the connection.\n";
        _is_running = false;
    } else {
        std::cerr << "Error reading from server.\n";
        _is_running = false;
    }
}

void Client::run()
{
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    while (_is_running) {
        if (poll(fds, 2, -1) == -1) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            std::string command;
            if (!std::getline(std::cin, command)) {
                _is_running = false;
            } else if (!command.empty()) {
                if (command == "exit") {
                    _is_running = false;
                } else {
                    send_command(command);
                }
            }
        }
        if (fds[1].revents & POLLIN) {
            receive_response();
        }
        if (fds[1].revents & (POLLHUP | POLLERR)) {
            std::cout << "Server disconnected.\n";
            _is_running = false;
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>\n";
        return 1;
    }
    std::string server_ip = argv[1];
    int port = atoi(argv[2]);
    Client client;
    client.create_socket();
    client.connect_to_server(server_ip, port);
    client.run();

    return 0;
}