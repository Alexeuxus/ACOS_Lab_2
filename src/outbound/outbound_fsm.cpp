#include "outbound_fsm.hpp"

#include <stdexcept>

namespace outbound {

namespace {

protocol::Command expected_response_command(protocol::Command req_cmd) {
    switch (req_cmd) {
        case protocol::Command::Ping:
            return protocol::Command::Pong;
        default:
            return req_cmd;
    }
}

} // namespace

OutboundFSM::OutboundFSM(SocketWriter& writer, std::chrono::milliseconds timeout)
    : writer_(writer), timeout_(timeout) {}

State OutboundFSM::state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

RequestResult OutboundFSM::send_request(protocol::Command cmd) {
    protocol::Message req{};
    std::string serialized;

    // 1. Состояние CREATE REQUEST: формируем запрос и присваиваем уникальный ID
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (disconnected_) {
            return {RequestStatus::Disconnected, std::nullopt, "Соединение разорвано"};
        }
        if (state_ != State::CreateRequest) {
            return {RequestStatus::ProtocolError, std::nullopt, "Автомат занят другим запросом"};
        }

        req.kind = protocol::MessageKind::Request;
        req.id = next_id_++;
        req.command = cmd;

        pending_request_ = req;
        received_response_.reset();
        state_ = State::WaitResponse;

        serialized = protocol::serialize(req);
    }

    // 2. Отправка через потокобезопасный SocketWriter (вне блокировки FSM)
    try {
        writer_.send_line(serialized);
    } catch (const std::exception& ex) {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_request_.reset();
        state_ = State::CreateRequest;
        return {RequestStatus::Disconnected, std::nullopt, ex.what()};
    }

    // 3. Состояние WAIT RESPONSE -> PROCESS RESPONSE
    std::unique_lock<std::mutex> lock(mutex_);
    const bool notified = cv_.wait_for(lock, timeout_, [this]() {
        return received_response_.has_value() || disconnected_;
    });

    state_ = State::ProcessResponse;

    if (disconnected_ && !received_response_.has_value()) {
        pending_request_.reset();
        state_ = State::CreateRequest;
        return {RequestStatus::Disconnected, std::nullopt, "Соединение закрыто удаленным узлом"};
    }

    if (!notified || !received_response_.has_value()) {
        pending_request_.reset();
        state_ = State::CreateRequest;
        return {RequestStatus::Timeout, std::nullopt, "Превышено время ожидания ответа"};
    }

    protocol::Message resp = *received_response_;
    received_response_.reset();
    pending_request_.reset();
    state_ = State::CreateRequest;

    // Проверка типа ответа (для PING ожидаем PONG)
    if (resp.command != expected_response_command(req.command)) {
        return {
            RequestStatus::ProtocolError,
            resp,
            "Неожиданный тип ответа на запрос id=" + std::to_string(req.id)
        };
    }

    return {RequestStatus::Success, resp, ""};
}

void OutboundFSM::on_response(const protocol::Message& msg) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Игнорируем ответы, если мы их сейчас не ждем или не совпал ID
    if (state_ != State::WaitResponse || !pending_request_.has_value()) {
        return;
    }
    if (msg.kind != protocol::MessageKind::Response || msg.id != pending_request_->id) {
        return;
    }

    received_response_ = msg;
    cv_.notify_one();
}

void OutboundFSM::on_disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    disconnected_ = true;
    cv_.notify_all();
}

} // namespace outbound
