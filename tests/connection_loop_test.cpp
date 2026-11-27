#include <atomic>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <sys/socket.h>

#include "inbound/connection_loop.hpp"
#include "common/fd_guard.hpp"
#include "common/socket_io.hpp"
#include "common/protocol.hpp"

int main() {
    int sockets[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        std::cerr << "socketpair failed\n";
        return 1;
    }

    FdGuard server(sockets[0]);
    FdGuard peer(sockets[1]);

    std::atomic<std::uint64_t> received_response_id{0};
    std::exception_ptr worker_error;

    std::thread worker([&]() {
        try {
            inbound::run_connection(
                server.get(),
                [&](const protocol::Message& message) {
                    if (message.kind == protocol::MessageKind::Response &&
                        message.command == protocol::Command::Pong) {
                        received_response_id.store(message.id);
                    }
                }
            );
        } catch (...) {
            worker_error = std::current_exception();
        }
    });

    try {
        LineReader reader(peer.get());

        const protocol::Message request{
            protocol::MessageKind::Request,
            42,
            protocol::Command::Ping
        };

        send_all(
            peer.get(),
            protocol::serialize(request) + "\n"
        );

        const auto response_line = reader.recv_line();

        if (!response_line) {
            throw std::runtime_error("No response received");
        }

        const protocol::Message response =
            protocol::parse(*response_line);

        if (response.kind != protocol::MessageKind::Response ||
            response.id != 42 ||
            response.command != protocol::Command::Pong) {
            throw std::runtime_error("Incorrect PONG response");
        }

        std::cout << "REQUEST -> RESPONSE: OK\n";

        const protocol::Message fake_response{
            protocol::MessageKind::Response,
            77,
            protocol::Command::Pong
        };

        send_all(
            peer.get(),
            protocol::serialize(fake_response) + "\n"
        );

        if (::shutdown(peer.get(), SHUT_WR) == -1) {
            throw std::runtime_error("shutdown failed");
        }

    } catch (const std::exception& ex) {
        ::shutdown(peer.get(), SHUT_WR);

        if (worker.joinable()) {
            worker.join();
        }

        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    worker.join();

    if (worker_error) {
        try {
            std::rethrow_exception(worker_error);
        } catch (const std::exception& ex) {
            std::cerr << "Worker failed: " << ex.what() << '\n';
            return 1;
        }
    }

    if (received_response_id.load() != 77) {
        std::cerr << "Response handler test failed\n";
        return 1;
    }

    std::cout << "RESPONSE dispatch: OK\n";
    std::cout << "Connection closed correctly: OK\n";
    std::cout << "Connection loop test passed!\n";

    return 0;
}