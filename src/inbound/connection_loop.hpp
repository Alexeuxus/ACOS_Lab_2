#pragma once

#include <functional>

#include "../common/protocol.hpp"
#include "../common/socket_writer.hpp"

namespace inbound {

using ResponseHandler =
    std::function<void(const protocol::Message&)>;

void run_connection(
    int fd,
    SocketWriter& writer,
    const ResponseHandler& on_response
);

} // namespace inbound