#pragma once

#include <cstdint>
#include <string>

#include "../common/fd_guard.hpp"

namespace outbound {

FdGuard connect_to_peer(const std::string& host, std::uint16_t port);

} // namespace outbound
