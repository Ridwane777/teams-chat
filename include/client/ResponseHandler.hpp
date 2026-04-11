#ifndef RESPONSE_HANDLER_HPP
#define RESPONSE_HANDLER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

class ResponseHandler {
public:
    static void execute(const std::string &opcode, const std::map<std::string, std::string> &fields);

private:
    static std::string _selfUuid;

    static void handle_error(const std::map<std::string, std::string> &fields);
    static void handle_event(const std::map<std::string, std::string> &fields);
    static void handle_ok(const std::map<std::string, std::string> &fields);
    static void handle_ok_user(const std::map<std::string, std::string> &fields);
    static void handle_ok_resource(const std::map<std::string, std::string> &fields);
};

#endif
