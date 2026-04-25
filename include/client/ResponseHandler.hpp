#ifndef RESPONSE_HANDLER_HPP
#define RESPONSE_HANDLER_HPP

#include <string>
#include <map>

class ResponseHandler {
public:
    static void execute(const std::string &opcode, const std::map<std::string, std::string> &fields);
    static void setPendingCommand(const std::string &cmd);

private:
    static std::string _selfUuid;
    static std::string _selfName;
    static std::string _pendingCommand;

    static void handle_error(const std::map<std::string, std::string> &fields);
    static void handle_event(const std::map<std::string, std::string> &fields);
    static void handle_ok(const std::map<std::string, std::string> &fields);
};

#endif
