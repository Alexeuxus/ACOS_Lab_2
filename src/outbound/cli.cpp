#include "cli.hpp"

#include <chrono>
#include <string_view>

namespace outbound {

namespace {

std::string trim_left(std::string_view sv) {
    const auto pos = sv.find_first_not_of(" \t\r\n");
    if (pos == std::string_view::npos) {
        return "";
    }
    return std::string(sv.substr(pos));
}

} // namespace

Cli::Cli(OutboundFSM& fsm, std::istream& in, std::ostream& out)
    : fsm_(fsm), in_(in), out_(out) {}

void Cli::stop() noexcept {
    running_.store(false);
}

void Cli::run() {
    running_.store(true);
    print_help();

    std::string line;
    while (running_.load()) {
        out_ << "> " << std::flush;
        if (!std::getline(in_, line)) {
            // Обработка EOF (Ctrl+D)
            out_ << "\nЗавершение работы CLI...\n";
            running_.store(false);
            break;
        }

        if (!running_.load()) {
            break;
        }

        handle_line(line);
    }
}

void Cli::handle_line(const std::string& line) {
    const std::string trimmed = trim_left(line);
    if (trimmed.empty()) {
        return;
    }

    if (trimmed == "/ping") {
        handle_ping();
    } else if (trimmed == "/history") {
        handle_history();
    } else if (trimmed == "/quit" || trimmed == "/exit") {
        out_ << "Выход из чата...\n";
        stop();
    } else if (trimmed == "/help") {
        print_help();
    } else if (trimmed == "/msg") {
        out_ << "[Ошибка] Укажите текст сообщения: /msg <текст>\n";
    } else if (trimmed.rfind("/msg ", 0) == 0) {
        const std::string text = trim_left(std::string_view(trimmed).substr(5));
        if (text.empty()) {
            out_ << "[Ошибка] Сообщение не может быть пустым.\n";
            return;
        }
        handle_msg(text);
    } else if (trimmed[0] == '/') {
        out_ << "[Ошибка] Неизвестная команда. Введите /help для списка команд.\n";
    } else {
        out_ << "[Подсказка] Используйте /msg <текст> для отправки сообщения или /help.\n";
    }
}

void Cli::handle_ping() {
    const auto start = std::chrono::steady_clock::now();
    const RequestResult res = fsm_.send_request(protocol::Command::Ping);
    const auto end = std::chrono::steady_clock::now();

    const auto rtt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    switch (res.status) {
        case RequestStatus::Success:
            out_ << "[PONG] Ответ получен (id=" << res.response->id
                 << ", rtt=" << rtt_ms << " мс)\n";
            break;
        case RequestStatus::Timeout:
            out_ << "[Таймаут] " << res.error_message << "\n";
            break;
        case RequestStatus::Disconnected:
            out_ << "[Отключено] " << res.error_message << "\n";
            stop();
            break;
        case RequestStatus::ProtocolError:
            out_ << "[Ошибка протокола] " << res.error_message << "\n";
            break;
    }
}

void Cli::handle_msg(const std::string& text) {
    // Будет подключено к OutboundFSM и ChatHistory после расширения protocol.hpp
    out_ << "[В разработке] Отправка сообщения \"" << text
         << "\" станет доступна после согласования команды MSG в протоколе.\n";
}

void Cli::handle_history() {
    // Будет подключено к ChatHistory на следующем этапе
    out_ << "[В разработке] Просмотр и запрос истории подключатся на этапе ChatHistory.\n";
}

void Cli::print_help() {
    out_ << "Доступные команды:\n"
         << "  /ping         — проверить связь с узлом\n"
         << "  /msg <текст>  — отправить сообщение\n"
         << "  /history      — показать историю переписки\n"
         << "  /quit         — выйти из программы\n";
}

} // namespace outbound
