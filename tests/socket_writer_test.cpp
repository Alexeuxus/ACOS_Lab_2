#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>
#include <sys/socket.h>

#include "common/fd_guard.hpp"
#include "common/socket_io.hpp"
#include "common/socket_writer.hpp"

int main() {
    int sockets[2];

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == -1) {
        std::cerr << "socketpair failed\n";
        return 1;
    }

    FdGuard sender(sockets[0]);
    FdGuard receiver(sockets[1]);

    SocketWriter writer(sender.get());
    LineReader reader(receiver.get());

    constexpr int thread_count = 4;
    constexpr int messages_per_thread = 10;
    constexpr int payload_size = 16 * 1024;

    const int total_messages =
        thread_count * messages_per_thread;

    std::vector<std::vector<std::string>> messages(thread_count);

    std::unordered_set<std::string> expected;

    for (int t = 0; t < thread_count; ++t) {
        for (int i = 0; i < messages_per_thread; ++i) {
            std::string message =
                "THREAD_" + std::to_string(t) +
                "_MESSAGE_" + std::to_string(i) + "|";

            message += std::string(
                payload_size,
                static_cast<char>('A' + t)
            );

            expected.insert(message);
            messages[t].push_back(std::move(message));
        }
    }

    std::vector<std::exception_ptr> errors(thread_count);
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&, t]() {
            try {
                for (const auto& message : messages[t]) {
                    writer.send_line(message);
                }
            } catch (...) {
                errors[t] = std::current_exception();

                ::shutdown(sender.get(), SHUT_WR);
            }
        });
    }

    bool valid = true;

    for (int i = 0; i < total_messages; ++i) {
        const auto line = reader.recv_line();

        if (!line) {
            valid = false;
            break;
        }

        if (expected.erase(*line) != 1) {
            valid = false;
        }
    }

    if (!valid) {
        ::shutdown(receiver.get(), SHUT_RDWR);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (const auto& error : errors) {
        if (error) {
            try {
                std::rethrow_exception(error);
            } catch (const std::exception& ex) {
                std::cerr << "Sender failed: "
                          << ex.what() << '\n';
                return 1;
            }
        }
    }

    if (!valid || !expected.empty()) {
        std::cerr << "Messages were corrupted or lost\n";
        return 1;
    }

    std::cout << "Threads: " << thread_count << '\n';
    std::cout << "Messages sent: " << total_messages << '\n';
    std::cout << "Message integrity: OK\n";
    std::cout << "SocketWriter test passed!\n";

    return 0;
}