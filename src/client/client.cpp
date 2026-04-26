#include "../../include/client/client.hpp"
#include "../../include/client/ClientCommand.hpp"
#include "../../include/client/ResponseHandler.hpp"

#include <sstream>
#include <stdexcept>
extern "C" {
#include "../../libs/myteams/logging_client.h"
}

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
    std::string cmd_with_newline = command + "\n";
    write(sockfd, cmd_with_newline.c_str(), cmd_with_newline.size());
}

void Client::receive_response()
{
    char buffer[MAX];
    ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        _readBuffer += buffer;
        size_t pos;
        while ((pos = _readBuffer.find('\n')) != std::string::npos) {
            std::string response = _readBuffer.substr(0, pos);
            _readBuffer.erase(0, pos + 1);
            if (!response.empty() && response.back() == '\r')
                response.pop_back();
            handle_response(response);
        }
    } else if (n == 0) {
        _is_running = false;
    } else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Error reading from server.\n";
            _is_running = false;
        }
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
            if (errno == EINTR)
                continue;
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
                    UserCommand::execute(sockfd, command);
                }
            }
        }
        if (fds[1].revents & POLLIN) {
            receive_response();
        }
        if (fds[1].revents & (POLLHUP | POLLERR)) {
            _is_running = false;
        }
    }
}

void Client::handle_response(const std::string &response)
{
    if (response.empty())
        return;

    size_t spacePos = response.find(' ');
    std::string opcode = (spacePos == std::string::npos) ? response : response.substr(0, spacePos);
    std::string payload = (spacePos == std::string::npos) ? "" : response.substr(spacePos + 1);

    try {
        std::map<std::string, std::string> fields = parse_payload(payload);
        for (auto &pair : fields) {
            pair.second = url_decode(pair.second);
        }
        ResponseHandler::execute(opcode, fields);
    } catch (const std::exception &e) {
        std::cerr << "Protocol Error: malformed payload - " << e.what() << "\n";
    }
}

std::string Client::url_decode(const std::string &str)
{
    std::string res;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            std::string hex = str.substr(i + 1, 2);
            char c = (char)strtol(hex.c_str(), nullptr, 16);
            res += c;
            i += 2;
        } else {
            res += str[i];
        }
    }
    return res;
}

std::string Client::url_encode(const std::string &str)
{
    static const char *hex = "0123456789ABCDEF";
    std::string res;
    for (unsigned char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            res += c;
        } else {
            res += '%';
            res += hex[c >> 4];
            res += hex[c & 0xf];
        }
    }
    return res;
}

std::map<std::string, std::string> Client::parse_payload(const std::string &payload)
{
    std::map<std::string, std::string> fields;
    if (payload.empty())
        return fields;

    std::stringstream ss(payload);
    std::string token;

    while (ss >> token) {
        size_t equalPos = token.find('=');
        if (equalPos == std::string::npos || equalPos == 0) {
            throw std::invalid_argument("missing or invalid key=value format in token: " + token);
        }
        std::string key = token.substr(0, equalPos);
        std::string value = token.substr(equalPos + 1);
        if (fields.count(key)) {
            throw std::invalid_argument("duplicate key: " + key);
        }
        fields[key] = value;
    }
    return fields;
}


int main(int argc, char *argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "USAGE: ./myteams_client ip port\n"
                  << " ip is the server IPv4 address\n"
                  << " port is the server port number" << std::endl;
        return 0;
    }
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>\n";
        return 84;
    }
    std::string server_ip = argv[1];
    int port = atoi(argv[2]);
    Client client;
    client.create_socket();
    client.connect_to_server(server_ip, port);
    client.run();
    return 0;
}