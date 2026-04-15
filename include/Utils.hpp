#ifndef UTILS_HPP
#define UTILS_HPP

#include "Types.hpp"

inline std::string uuidToString(const uuid_t &uuid)
{
    char buffer[37] = {0};

    uuid_unparse_lower(uuid, buffer);
    return std::string(buffer);
}


#endif