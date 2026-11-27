#pragma once

#include <optional>
#include <string>
#include <string_view>

// todo: допилить реализацию

// send data through socket
void send_all(int fd, std::string_view data);

// gets lines that parts with '\n', saves unread messages between calls
class LineReader {
public:
    explicit LineReader(int fd) noexcept;

    std::optional<std::string> recv_line();

private:
    int fd_;
    std::string buffer_;
};
