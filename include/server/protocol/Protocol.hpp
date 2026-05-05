#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP
#include <string>
#include <unordered_map>
#include <optional>

namespace ClientOp
{
    constexpr const char *LOGIN = "LOG";
    constexpr const char *LOGOUT = "OUT";
    constexpr const char *USERS_LIST = "ULS";
    constexpr const char *USER_GET = "UGT";
    // Not named MSG_SEND: macOS sys/socket.h defines MSG_SEND as a macro.
    constexpr const char *SEND_MESSAGE = "SND";
    constexpr const char *MSG_LIST = "MLS";
    constexpr const char *TEAM_CREATE = "TCR";
    constexpr const char *TEAM_SUB = "TSB";
    constexpr const char *TEAM_UNSUB = "TUS";
    constexpr const char *TEAM_SUBBED = "TSD";
    constexpr const char *CHAN_CREATE = "CCR";
    constexpr const char *THREAD_CREATE = "HCR";
    constexpr const char *COMMENT_CREATE = "OCR";
    constexpr const char *LIST = "LST";
    constexpr const char *INFO = "INF";
    constexpr const char *USE = "USE";
}

namespace ServerOp
{
    constexpr const char *OK = "OK";
    constexpr const char *ERR = "ERR";
    constexpr const char *EVT = "EVT";
}

namespace EventType
{
    constexpr const char *USER_ONLINE = "USER_ONLINE";
    constexpr const char *USER_OFFLINE = "USER_OFFLINE";
    constexpr const char *TEAM_CREATED = "TEAM_CREATED";
    constexpr const char *CHANNEL_CREATED = "CHANNEL_CREATED";
    constexpr const char *THREAD_CREATED = "THREAD_CREATED";
    constexpr const char *COMMENT_CREATED = "COMMENT_CREATED";
    constexpr const char *MSG_RECEIVED = "MSG_RECEIVED";
}

namespace ErrCode
{
    constexpr int OK = 200;
    constexpr int BAD_REQUEST = 400;
    constexpr int UNAUTHORIZED = 401;
    constexpr int FORBIDDEN = 403;
    constexpr int NOT_FOUND = 404;
    constexpr int CONFLICT = 409;
    constexpr int INTERNAL = 500;
}

constexpr size_t MAX_FRAME_SIZE = 4096;

struct Frame
{
    std::string opcode;
    std::unordered_map<std::string, std::string> fields;

    bool has(const std::string &key) const
    {
        return fields.count(key) > 0;
    }

    const std::string &get(const std::string &key) const
    {
        static const std::string empty;
        auto it = fields.find(key);
        return it != fields.end() ? it->second : empty;
    }

    std::optional<std::string> opt(const std::string &key) const
    {
        auto it = fields.find(key);
        if (it == fields.end())
            return std::nullopt;
        return it->second;
    }
};

class Protocol
{
public:
    std::string encode(const std::string &s);
    std::string decode(const std::string &s);
    std::optional<Frame> parse(const std::string &line);
    std::string serialise(const Frame &frame);
    std::string ok(std::initializer_list<
                              std::pair<std::string, std::string>>
                              fields = {});

    std::string err(int code, const std::string &reason);
    std::string evt(const std::string &event,
                     std::initializer_list<
                         std::pair<std::string, std::string>>
                         fields = {});
};

#endif