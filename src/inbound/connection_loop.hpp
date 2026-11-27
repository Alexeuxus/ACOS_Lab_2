#pragma once

#include <functional>

#include "../common/protocol.hpp"

namespace inbound {

using ResponseHandler =
    std::function<void(const protocol::Message&)>;

void run_connection(
    int fd,
    const ResponseHandler& on_response
);

} // namespace inbound