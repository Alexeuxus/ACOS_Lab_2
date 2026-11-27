
#include "socket_io.hpp"

#include <cerrno>
#include <system_error>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdexcept>
#include <string>

void send_all(int fd, std::string_view data) {
    std::size_t total_sent = 0;

    while (total_sent < data.size()) {
        const ssize_t sent = ::send(
            fd,
            data.data() + total_sent,
            data.size() - total_sent,
            MSG_NOSIGNAL
        );

        if (sent > 0) {
            total_sent += static_cast<std::size_t>(sent);
            continue;
        }

        if (sent == -1 && errno == EINTR) {
            continue;
        }

        if (sent == 0) {
            throw std::system_error(
                std::make_error_code(std::errc::io_error),
                "send returned 0"
            );
        }

        throw std::system_error(
            errno,
            std::generic_category(),
            "send"
        );
    }
}

LineReader::LineReader(int fd) noexcept
    : fd_(fd) {}

std::optional<std::string> LineReader::recv_line() {
    constexpr std::size_t max_line_size = 64 * 1024;
    constexpr std::size_t chunk_size = 4096;

    while (true) {
        const std::size_t newline_pos = buffer_.find('\n');

        if (newline_pos != std::string::npos) {
            if (newline_pos > max_line_size) {
                throw std::length_error("Incoming message is too large");
            }

            std::string line = buffer_.substr(0, newline_pos);
            buffer_.erase(0, newline_pos + 1);

            return line;
        }

        if (buffer_.size() > max_line_size) {
            throw std::length_error("Incoming message is too large");
        }

        char chunk[chunk_size];

        ssize_t received;

        do {
            received = ::recv(fd_, chunk, sizeof(chunk), 0);
        } while (received == -1 && errno == EINTR);

        if (received > 0) {
            buffer_.append(
                chunk,
                static_cast<std::size_t>(received)
            );

            continue;
        }

        if (received == 0) {
            if (buffer_.empty()) {
                return std::nullopt;
            }

            throw std::runtime_error(
                "Connection closed before message was complete"
            );
        }

        throw std::system_error(
            errno,
            std::generic_category(),
            "recv"
        );
    }
}