#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace protocol {

enum class MessageKind {
    Request,
    Response
};

enum class Command {
    Ping,
    Pong
};

struct Message {
    MessageKind kind;
    std::uint64_t id;
    Command command;
};

std::string serialize(const Message& message);

Message parse(std::string_view line);

} // namespace protocol