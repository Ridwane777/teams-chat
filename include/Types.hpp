#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <ctime>
#include <vector>
#include <optional>
#include <string>
#include <functional>
#include <algorithm>
#include <stdexcept>

constexpr size_t MAX_NAME_LENGTH = 32;
constexpr size_t MAX_DESCRIPTION_LENGTH = 255;
constexpr size_t MAX_BODY_LENGTH = 512;
constexpr size_t UUID_STR_LEN = 37;

using uuid_t = std::string;

#endif