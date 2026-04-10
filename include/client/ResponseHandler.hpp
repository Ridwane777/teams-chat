#ifndef RESPONSE_HANDLER_HPP_
#define RESPONSE_HANDLER_HPP_

#include <iostream>
#include <string>
#include <map>
#include <vector>

class ResponseHandler {
public:
    typedef void (*handler_t)(const std::vector<std::string> &args);
    static void execute(const std::string &code, const std::vector<std::string> &args);

private:
    static std::map<std::string, handler_t> _responses;
    static void initMap();

    // Event Handlers
    static void handleLoggedIn(const std::vector<std::string> &args);
    static void handleLoggedOut(const std::vector<std::string> &args);
    static void handlePrivateMessageReceived(const std::vector<std::string> &args);
    
    // Print Handlers
    static void handlePrintUsers(const std::vector<std::string> &args);
    static void handlePrintTeams(const std::vector<std::string> &args);
    
    // Error Handlers
    static void handleUnknownTeam(const std::vector<std::string> &args);
    static void handleUnknownChannel(const std::vector<std::string> &args);
    static void handleUnknownThread(const std::vector<std::string> &args);
    static void handleUnknownUser(const std::vector<std::string> &args);
    static void handleUnauthorized(const std::vector<std::string> &args);
    static void handleAlreadyExist(const std::vector<std::string> &args);
};

#endif /* !RESPONSE_HANDLER_HPP_ */
