#pragma once

#include <atomic>
#include <iostream>
#include <string>

#include "outbound_fsm.hpp"

namespace outbound {

class Cli {
public:
    explicit Cli(
        OutboundFSM& fsm,
        std::istream& in = std::cin,
        std::ostream& out = std::cout
    );

    // Запускает блокирующий цикл чтения команд пользователя из консоли.
    void run();

    // Позволяет остановить цикл извне (например, при разрыве соединения).
    void stop() noexcept;

private:
    void handle_line(const std::string& line);
    void handle_ping();
    void handle_msg(const std::string& text);
    void handle_history();
    void print_help();

    OutboundFSM& fsm_;
    std::istream& in_;
    std::ostream& out_;
    std::atomic<bool> running_{false};
};

} // namespace outbound
