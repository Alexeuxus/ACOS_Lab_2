#include <cerrno>
#include <iostream>
#include <string>
#include <system_error>
#include <sys/socket.h>

#include "common/fd_guard.hpp"
#include "common/socket_io.hpp"

int main() {
    int sockets[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        std::cerr << "socketpair failed: "
                  << std::system_error(
                         errno, std::generic_category()).what()
                  << '\n';
        return 1;
    }

    FdGuard sender(sockets[0]);
    FdGuard receiver(sockets[1]);

    try {
        const std::string first_message = "REQUEST 1 PING";
        const std::string second_message = "REQUEST 2 PING";

        send_all(sender.get(),
                 first_message + "\n" + second_message + "\n");

        if (::shutdown(sender.get(), SHUT_WR) == -1) {
            throw std::system_error(
                errno, std::generic_category(), "shutdown"
            );
        }

        LineReader reader(receiver.get());

        const auto first = reader.recv_line();

        if (!first || *first != first_message) {
            throw std::runtime_error("First message mismatch");
        }

        const auto second = reader.recv_line();

        if (!second || *second != second_message) {
            throw std::runtime_error("Second message mismatch");
        }

        const auto third = reader.recv_line();

        if (third.has_value()) {
            throw std::runtime_error("Expected EOF");
        }

        std::cout << "Received: " << *first << '\n';
        std::cout << "Received: " << *second << '\n';
        std::cout << "EOF handled correctly\n";
        std::cout << "LineReader test passed!\n";

    } catch (const std::exception& ex) {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}