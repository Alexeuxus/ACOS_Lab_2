#pragma once

#include <mutex>
#include <string_view>

class SocketWriter {
public:
    explicit SocketWriter(int fd);

    SocketWriter(const SocketWriter&) = delete;
    SocketWriter& operator=(const SocketWriter&) = delete;

    SocketWriter(SocketWriter&&) = delete;
    SocketWriter& operator=(SocketWriter&&) = delete;

    void send_line(std::string_view line);

private:
    int fd_;
    std::mutex send_mutex_;
};