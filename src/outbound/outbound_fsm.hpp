#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>

#include "../common/protocol.hpp"
#include "../common/socket_writer.hpp"

namespace outbound {

enum class State {
    CreateRequest,
    WaitResponse,
    ProcessResponse
};

enum class RequestStatus {
    Success,
    Timeout,
    Disconnected,
    ProtocolError
};

struct RequestResult {
    RequestStatus status;
    std::optional<protocol::Message> response;
    std::string error_message;
};

class OutboundFSM {
public:
    explicit OutboundFSM(
        SocketWriter& writer,
        std::chrono::milliseconds timeout = std::chrono::seconds(3)
    );

    OutboundFSM(const OutboundFSM&) = delete;
    OutboundFSM& operator=(const OutboundFSM&) = delete;

    // Инициирует отправку запроса и синхронно ждет сопоставленный ответ.
    RequestResult send_request(protocol::Command cmd);

    // Колбэк для передачи в inbound::run_connection (ResponseHandler).
    void on_response(const protocol::Message& msg);

    // Вызывается при закрытии соединения циклом чтения, чтобы разблокировать ожидание.
    void on_disconnect();

    State state() const;

private:
    SocketWriter& writer_;
    std::chrono::milliseconds timeout_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;

    State state_ = State::CreateRequest;
    std::uint64_t next_id_ = 1;
    bool disconnected_ = false;

    std::optional<protocol::Message> pending_request_;
    std::optional<protocol::Message> received_response_;
};

} // namespace outbound
