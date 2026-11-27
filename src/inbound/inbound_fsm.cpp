#include <stdexcept>
#include <string>

#include "inbound_fsm.hpp"

namespace inbound {

State InboundFSM::state() const noexcept {
    return state_;
}

void InboundFSM::transition_to(State next) {
    bool allowed = false;

    switch (state_) {
        case State::WaitingRequest:
            allowed =
                next == State::ProcessingRequest ||
                next == State::Stopped;
            break;

        case State::ProcessingRequest:
            allowed =
                next == State::SendingResponse ||
                next == State::Error;
            break;

        case State::SendingResponse:
            allowed =
                next == State::WaitingRequest ||
                next == State::Error;
            break;

        case State::Error:
            allowed =
                next == State::WaitingRequest ||
                next == State::Stopped;
            break;

        case State::Stopped:
            allowed = false;
            break;
    }

    if (!allowed) {
        throw std::logic_error(
            "Invalid FSM state transition"
        );
    }

    state_ = next;
}

void InboundFSM::handle_request(
    SocketWriter& writer,
    const protocol::Message& request
) {
    if (state_ != State::WaitingRequest) {
        throw std::logic_error(
            "FSM is not waiting for a request"
        );
    }

    transition_to(State::ProcessingRequest);

    if (request.kind != protocol::MessageKind::Request ||
        request.command != protocol::Command::Ping) {

        transition_to(State::Error);
        transition_to(State::WaitingRequest);

        throw std::invalid_argument(
            "Unsupported incoming request"
        );
    }

    const protocol::Message response{
        protocol::MessageKind::Response,
        request.id,
        protocol::Command::Pong
    };

    transition_to(State::SendingResponse);

    try {
        writer.send_line(protocol::serialize(response));

    } catch (...) {
        transition_to(State::Error);
        transition_to(State::Stopped);
        throw;
    }

    transition_to(State::WaitingRequest);
}

} // namespace inbound