#ifndef SERVERRESPONSE_HPP
#define SERVERRESPONSE_HPP

#include "ServerState.hpp"
#include "protocol/Protocol.hpp"
#include "../libs/myteams/logging_client.h"

class ServerResponse
{
public:
    ServerResponse() = default;
    ~ServerResponse() = default;

    ServerResponse(const ServerResponse &) = delete;
    ServerResponse &operator=(const ServerResponse &) = delete;
    ServerResponse(ServerResponse &&) = default;
    ServerResponse &operator=(ServerResponse &&) = default;

    static std::string ok(const std::map<std::string, std::string> &data = {});
    static std::string err(int code, const std::string &message);
};

#endif