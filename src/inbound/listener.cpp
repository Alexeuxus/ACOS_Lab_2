#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <sys/socket.h>
#include <netinet/in.h>

#include "listener.hpp"

namespace network {

FdGuard create_listener(std::uint16_t port, int backlog) {
    if (backlog <= 0) {
        throw std::invalid_argument("Backlog must be positive");
    }

    // creates tcp socket
    const int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (raw_fd == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "socket"
        );
    }

    FdGuard listener(raw_fd);

    // allows reuse of address
    const int reuse = 1;

    if (::setsockopt(
            listener.get(),
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)
        ) == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "setsockopt"
        );
    }

    // setting up address and port
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    // binds socket to address and port
    if (::bind(
            listener.get(),
            reinterpret_cast<const sockaddr*>(&address),
            sizeof(address)
        ) == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "bind"
        );
    }

    // change socket to begin listen
    if (::listen(listener.get(), backlog) == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "listen"
        );
    }

    return listener;
}

FdGuard accept_connection(int listening_fd) {
    int client_fd;

    // if connection is interrupted - repeats accept()
    do {
        client_fd = ::accept(listening_fd, nullptr, nullptr);
    } while (client_fd == -1 && errno == EINTR);

    if (client_fd == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "accept"
        );
    }

    return FdGuard(client_fd);
}

} // namespace network