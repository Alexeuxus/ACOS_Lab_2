#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

#include "inbound/inbound_fsm.hpp"
#include "common/fd_guard.hpp"
#include "common/socket_io.hpp"
#include "common/protocol.hpp"
#include "common/socket_writer.hpp"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    int sockets[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        std::cerr << "socketpair failed\n";
        return 1;
    }

    FdGuard server(sockets[0]);
    FdGuard peer(sockets[1]);

    SocketWriter writer(server.get());

    try {
        inbound::InboundFSM fsm;

        LineReader peer_reader(peer.get());

        require(
            fsm.state() == inbound::State::WaitingRequest,
            "Incorrect initial state"
        );

        const protocol::Message request{
            protocol::MessageKind::Request,
            42,
            protocol::Command::Ping
        };

        fsm.handle_request(writer, request);

        const auto response_text = peer_reader.recv_line();

        require(
            response_text.has_value(),
            "Response was not received"
        );

        const protocol::Message response =
            protocol::parse(*response_text);

        require(
            response.kind == protocol::MessageKind::Response,
            "Incorrect response kind"
        );

        require(
            response.id == 42,
            "Incorrect response ID"
        );

        require(
            response.command == protocol::Command::Pong,
            "Incorrect response command"
        );

        std::cout << "PING -> PONG: OK\n";

        require(
            fsm.state() == inbound::State::WaitingRequest,
            "FSM did not return to WaitingRequest"
        );

        std::cout << "State after successful request: OK\n";

        const protocol::Message invalid_request{
            protocol::MessageKind::Response,
            43,
            protocol::Command::Pong
        };

        bool error_caught = false;

        try {
            fsm.handle_request(writer, invalid_request);
        } catch (const std::invalid_argument&) {
            error_caught = true;
        }

        require(
            error_caught,
            "Invalid request was not rejected"
        );

        std::cout << "Invalid request rejection: OK\n";

        require(
            fsm.state() == inbound::State::WaitingRequest,
            "FSM did not recover after invalid request"
        );

        std::cout << "State recovery: OK\n";

        const protocol::Message second_request{
            protocol::MessageKind::Request,
            44,
            protocol::Command::Ping
        };

        fsm.handle_request(writer, second_request);

        const auto second_response_text =
            peer_reader.recv_line();

        require(
            second_response_text.has_value(),
            "Second response was not received"
        );

        const protocol::Message second_response =
            protocol::parse(*second_response_text);

        require(
            second_response.id == 44 &&
            second_response.command == protocol::Command::Pong,
            "Second response is incorrect"
        );

        std::cout << "Second request: OK\n";
        std::cout << "Inbound FSM test passed!\n";

    } catch (const std::exception& ex) {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}