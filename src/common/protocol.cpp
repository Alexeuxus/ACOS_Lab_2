#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>

#include "protocol.hpp"

namespace protocol {

namespace {

// extracts word from line
std::string_view next_token(std::string_view& input) noexcept {
    while (!input.empty() && input.front() == ' ') {
        input.remove_prefix(1);
    }

    if (input.empty()) {
        return {};
    }

    const std::size_t separator = input.find(' ');

    if (separator == std::string_view::npos) {
        const std::string_view token = input;
        input = {};
        return token;
    }

    const std::string_view token = input.substr(0, separator);
    input.remove_prefix(separator);

    return token;
}

} // namespace

// forms message into a string
std::string serialize(const Message& message) {
    if (message.id == 0) {
        throw std::invalid_argument("Message ID cannot be zero");
    }

    const char* kind_text = nullptr;
    const char* command_text = nullptr;

    switch (message.kind) {
        case MessageKind::Request:
            if (message.command != Command::Ping) {
                throw std::invalid_argument(
                    "Invalid command for REQUEST"
                );
            }

            kind_text = "REQ";
            command_text = "PING";
            break;

        case MessageKind::Response:
            if (message.command != Command::Pong) {
                throw std::invalid_argument(
                    "Invalid command for RESPONSE"
                );
            }

            kind_text = "RES";
            command_text = "PONG";
            break;

        default:
            throw std::invalid_argument("Unknown message kind");
    }

    return std::string(kind_text) + " " +
           std::to_string(message.id) + " " +
           command_text;
}

// forms string into a message
Message parse(std::string_view line) {
    if (line.find('\n') != std::string_view::npos ||
        line.find('\r') != std::string_view::npos) {
        throw std::invalid_argument(
            "Message must contain exactly one line"
        );
    }

    const std::string_view kind_token = next_token(line);
    const std::string_view id_token = next_token(line);
    const std::string_view command_token = next_token(line);
    const std::string_view extra_token = next_token(line);

    if (kind_token.empty() ||
        id_token.empty() ||
        command_token.empty() ||
        !extra_token.empty()) {
        throw std::invalid_argument(
            "Invalid message format"
        );
    }

    MessageKind kind;

    if (kind_token == "REQ") {
        kind = MessageKind::Request;
    } else if (kind_token == "RES") {
        kind = MessageKind::Response;
    } else {
        throw std::invalid_argument(
            "Unknown message kind: " + std::string(kind_token)
        );
    }

    std::uint64_t id = 0;

    const auto result = std::from_chars(
        id_token.data(),
        id_token.data() + id_token.size(),
        id,
        10
    );

    if (result.ec != std::errc{} ||
        result.ptr != id_token.data() + id_token.size() ||
        id == 0) {
        throw std::invalid_argument("Invalid message ID");
    }

    Command command;

    if (command_token == "PING") {
        command = Command::Ping;
    } else if (command_token == "PONG") {
        command = Command::Pong;
    } else {
        throw std::invalid_argument(
            "Unknown command: " + std::string(command_token)
        );
    }

    // type and command validation
    if (kind == MessageKind::Request &&
        command != Command::Ping) {
        throw std::invalid_argument(
            "REQUEST can contain only PING"
        );
    }

    if (kind == MessageKind::Response &&
        command != Command::Pong) {
        throw std::invalid_argument(
            "RESPONSE can contain only PONG"
        );
    }

    return Message{kind, id, command};
}

} // namespace protocol