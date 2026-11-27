#pragma once

#include <cstdint>

#include "../common/fd_guard.hpp"

namespace network {
FdGuard create_listener(std::uint16_t port, int backlog = 10);
FdGuard accept_connection(int listening_fd);
} // namespace network