#include <stdexcept>

#include "connection_loop.hpp"
#include "../common/socket_io.hpp"
#include "inbound_fsm.hpp"

namespace inbound {

void run_connection(
    int fd,
    SocketWriter& writer,
    const ResponseHandler& on_response
) {
    LineReader reader(fd);

    InboundFSM fsm;

    while (true) {
        const auto line = reader.recv_line();
        if (!line.has_value()) {
            return;
        }

        const protocol::Message message =
            protocol::parse(*line);

        switch (message.kind) {

            case protocol::MessageKind::Request:
                fsm.handle_request(writer, message);
                break;

            case protocol::MessageKind::Response:
                if (!on_response) {
                    throw std::logic_error(
                        "Response handler is not configured"
                    );
                }

                on_response(message);
                break;

            default:
                throw std::invalid_argument(
                    "Unknown message kind"
                );
        }
    }
}

} // namespace inbound