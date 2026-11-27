#include <cerrno>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <netinet/in.h>
#include <sys/socket.h>

#include "inbound/listener.hpp"
#include "common/socket_io.hpp"

int main() {
    try {
        FdGuard listener = network::create_listener(0);

        sockaddr_in server_address{};
        socklen_t address_length = sizeof(server_address);

        if (::getsockname(
                listener.get(),
                reinterpret_cast<sockaddr*>(&server_address),
                &address_length
            ) == -1) {
            throw std::system_error(
                errno,
                std::generic_category(),
                "getsockname"
            );
        }

        std::cout << "Server listening on port: "
                  << ntohs(server_address.sin_port) << '\n';

        const int raw_client = ::socket(AF_INET, SOCK_STREAM, 0);

        if (raw_client == -1) {
            throw std::system_error(
                errno,
                std::generic_category(),
                "socket"
            );
        }

        FdGuard client(raw_client);

        sockaddr_in client_address{};

        client_address.sin_family = AF_INET;
        client_address.sin_port = server_address.sin_port;
        client_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        if (::connect(
                client.get(),
                reinterpret_cast<const sockaddr*>(&client_address),
                sizeof(client_address)
            ) == -1) {
            throw std::system_error(
                errno,
                std::generic_category(),
                "connect"
            );
        }

        std::cout << "Client connected!\n";

        FdGuard accepted = network::accept_connection(listener.get());

        std::cout << "Server accepted connection!\n";

        send_all(client.get(), "PING\n");

        LineReader server_reader(accepted.get());

        const auto request = server_reader.recv_line();

        if (!request || *request != "PING") {
            throw std::runtime_error("PING reception failed");
        }

        std::cout << "Server received: " << *request << '\n';

        send_all(accepted.get(), "PONG\n");

        LineReader client_reader(client.get());

        const auto response = client_reader.recv_line();

        if (!response || *response != "PONG") {
            throw std::runtime_error("PONG reception failed");
        }

        std::cout << "Client received: " << *response << '\n';

        std::cout << "Listener test passed!\n";

    } catch (const std::exception& ex) {
        std::cerr << "Test failed: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}