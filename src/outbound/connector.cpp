#include "connector.hpp"

#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>

namespace outbound {

FdGuard connect_to_peer(const std::string& host, std::uint16_t port) {
    struct addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* result = nullptr;
    const std::string port_str = std::to_string(port);

    int status = ::getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result);
    if (status != 0) {
        throw std::runtime_error(
            "Failed to resolve address " + host + ":" + port_str + " (" + ::gai_strerror(status) + ")"
        );
    }

    FdGuard connected_fd;
    int last_errno = 0;

    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        int raw_fd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (raw_fd < 0) {
            last_errno = errno;
            continue;
        }

        FdGuard candidate(raw_fd);

        if (::connect(candidate.get(), rp->ai_addr, rp->ai_addrlen) == 0) {
            connected_fd = std::move(candidate);
            break;
        }

        last_errno = errno;
    }

    ::freeaddrinfo(result);

    if (!connected_fd.valid()) {
        throw std::runtime_error(
            "Failed to connect to " + host + ":" + port_str + " (" + std::strerror(last_errno) + ")"
        );
    }

    return connected_fd;
}

} // namespace outbound
