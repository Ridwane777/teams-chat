#include "../../../include/server/protocol/Protocol.hpp"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <stdexcept>


std::string Protocol::encode(const std::string &s)
{
    std::ostringstream out;
    out << std::hex << std::uppercase;

    for (unsigned char c : s)
    {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            out << c;
        }
        else
        {
            out << '%' << std::setw(2) << std::setfill('0')
                << static_cast<int>(c);
        }
    }
    return out.str();
}

std::string Protocol::decode(const std::string &s)
{
    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '%' && i + 2 < s.size() && std::isxdigit(s[i + 1]) && std::isxdigit(s[i + 2]))
        {
            int val = std::stoi(s.substr(i + 1, 2), nullptr, 16);
            out += static_cast<char>(val);
            i += 2;
        }
        else
        {
            out += s[i];
        }
    }
    return out;
}

std::optional<Frame> Protocol::parse(const std::string &raw)
{
    std::string line = raw;
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();

    if (line.size() > MAX_FRAME_SIZE)
        return std::nullopt;

    size_t sp = line.find(' ');
    Frame f;
    f.opcode = (sp == std::string::npos) ? line : line.substr(0, sp);

    if (f.opcode.size() != 2 && f.opcode.size() != 3)
        return std::nullopt;
    for (char c : f.opcode)
        if (!std::isupper(static_cast<unsigned char>(c)))
            return std::nullopt;

    if (sp == std::string::npos)
        return f;

    std::istringstream ss(line.substr(sp + 1));
    std::string token;
    while (ss >> token)
    {
        size_t eq = token.find('=');
        if (eq == std::string::npos || eq == 0)
            return std::nullopt;
        std::string key = token.substr(0, eq);
        std::string value = decode(token.substr(eq + 1));
        f.fields[key] = std::move(value);
    }
    return f;
}

std::string Protocol::serialise(const Frame &frame)
{
    std::ostringstream out;
    out << frame.opcode;

    for (const auto &[k, v] : frame.fields)
        out << ' ' << k << '=' << encode(v);

    return out.str();
}

std::string Protocol::ok(
    std::initializer_list<std::pair<std::string, std::string>> fields)
{
    Frame f;
    f.opcode = ServerOp::OK;
    for (const auto &[k, v] : fields)
        f.fields[k] = v;
    return serialise(f) + "\n";
}

std::string Protocol::err(int code, const std::string &reason)
{
    Frame f;
    f.opcode = ServerOp::ERR;
    f.fields["code"] = std::to_string(code);
    f.fields["reason"] = reason;
    return serialise(f) + "\n";
}

std::string Protocol::evt(const std::string &event,
                          std::initializer_list<std::pair<std::string, std::string>> fields)
{
    Frame f;
    f.opcode = ServerOp::EVT;
    f.fields["event"] = event;
    for (const auto &[k, v] : fields)
        f.fields[k] = v;
    return serialise(f) + "\n";
}
