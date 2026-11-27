#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "common/protocol.hpp"

namespace {

void expect_invalid(std::string_view line) {
    try {
        (void)protocol::parse(line);
    } catch (const std::invalid_argument&) {
        return;
    }

    throw std::runtime_error(
        "Expected invalid message: " + std::string(line)
    );
}

} // namespace

int main() {
    try {
        const protocol::Message request{
            protocol::MessageKind::Request,
            1,
            protocol::Command::Ping
        };

        const std::string request_text =
            protocol::serialize(request);

        if (request_text != "REQ 1 PING") {
            throw std::runtime_error("Request serialization failed");
        }

        const protocol::Message parsed_request =
            protocol::parse(request_text);

        if (parsed_request.kind != protocol::MessageKind::Request ||
            parsed_request.id != 1 ||
            parsed_request.command != protocol::Command::Ping) {
            throw std::runtime_error("Request parsing failed");
        }

        const protocol::Message response{
            protocol::MessageKind::Response,
            5,
            protocol::Command::Pong
        };

        const std::string response_text =
            protocol::serialize(response);

        if (response_text != "RES 5 PONG") {
            throw std::runtime_error("Response serialization failed");
        }

        const protocol::Message parsed_response =
            protocol::parse(response_text);

        if (parsed_response.kind != protocol::MessageKind::Response ||
            parsed_response.id != 5 ||
            parsed_response.command != protocol::Command::Pong) {
            throw std::runtime_error("Response parsing failed");
        }

        expect_invalid("");
        expect_invalid("HELLO 1 PING");
        expect_invalid("REQ abc PING");
        expect_invalid("REQ 0 PING");
        expect_invalid("REQ 1 PONG");
        expect_invalid("RES 1 PING");
        expect_invalid("REQ 1 PING EXTRA");

        std::cout << "Request serialization: OK\n";
        std::cout << "Request parsing: OK\n";
        std::cout << "Response serialization: OK\n";
        std::cout << "Response parsing: OK\n";
        std::cout << "Invalid messages handling: OK\n";
        std::cout << "Protocol test passed!\n";

    } catch (const std::exception& ex) {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
