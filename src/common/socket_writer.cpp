#include <stdexcept>
#include <string>

#include "socket_writer.hpp"
#include "socket_io.hpp"

SocketWriter::SocketWriter(int fd)
    : fd_(fd) {
    if (fd_ < 0) {
        throw std::invalid_argument(
            "Invalid socket descriptor"
        );
    }
}

void SocketWriter::send_line(std::string_view line) {
    if (line.find_first_of("\r\n") != std::string_view::npos) {
        throw std::invalid_argument(
            "Message contains a line separator"
        );
    }

    std::string frame(line);
    frame.push_back('\n');

    std::lock_guard<std::mutex> lock(send_mutex_);

    send_all(fd_, frame);
}