#ifndef LOGGING_SPY_HPP
#define LOGGING_SPY_HPP

#include <string>
#include <unordered_map>

void reset_logging_spy();
int logging_call_count(const std::string &name);

#endif
