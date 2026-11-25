#include <iostream>
#include <stdexcept>
#include <string>
#include <cerrno>
#include <fcntl.h>

#include "common/fd_guard.hpp"

namespace {

struct Config {
    int listen_port = 9001;
    std::string peer_host = "127.0.0.1";
    int peer_port = 9002;
};

int parse_port(const std::string& raw) {
    size_t idx = 0;
    const int value = std::stoi(raw, &idx, 10);
    if (idx != raw.size() || value <= 0 || value > 65535) {
        throw std::invalid_argument("Некорректный порт: " + raw);
    }
    return value;
}

Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--listen-port" && i + 1 < argc) {
            cfg.listen_port = parse_port(argv[++i]);
        } else if (arg == "--peer-host" && i + 1 < argc) {
            cfg.peer_host = argv[++i];
        } else if (arg == "--peer-port" && i + 1 < argc) {
            cfg.peer_port = parse_port(argv[++i]);
        } else {
            throw std::invalid_argument("Неизвестный или неполный аргумент: " + arg);
        }
    }
    return cfg;
}

bool test_fd_guard() {
    const int raw_fd = ::open("/dev/null", O_RDONLY);

    if (raw_fd < 0) {
        return false;
    }

    const int saved_fd = raw_fd;

    {
        FdGuard guarded_fd(raw_fd);

        if (!guarded_fd.valid()) {
            return false;
        }

        if (::fcntl(guarded_fd.get(), F_GETFD) == -1) {
            return false;
        }
    }

    errno = 0;

    if (::fcntl(saved_fd, F_GETFD) != -1 || errno != EBADF) {
        return false;
    }

    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        const Config cfg = parse_args(argc, argv);

        if (!test_fd_guard()) {
            std::cerr << "FdGuard test failed\n";
            return 1;
        }

        std::cout << "FdGuard test passed!\n";
        
        std::cout << "Peer starting: listen_port=" << cfg.listen_port
                  << ", peer=" << cfg.peer_host << ":" << cfg.peer_port << "\n";

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка запуска: " << ex.what() << "\n";
        return 1;
    }
}