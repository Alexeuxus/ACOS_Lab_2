#pragma once

#include "../common/protocol.hpp"
#include "../common/socket_writer.hpp"

namespace inbound {

enum class State {
    WaitingRequest,
    ProcessingRequest,
    SendingResponse,
    Error,
    Stopped
};

class InboundFSM {
public:
    void handle_request(
    SocketWriter& writer,
    const protocol::Message& request
    );

    State state() const noexcept;

private:
    void transition_to(State next);

    State state_ = State::WaitingRequest;
};

} // namespace inbound