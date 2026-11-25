
#pragma once

#include <unistd.h>

class FdGuard {
public:
    explicit FdGuard(int fd = -1) noexcept
        : fd_(fd) {}

    ~FdGuard() {
        reset();
    }

    FdGuard(const FdGuard&) = delete;
    FdGuard& operator=(const FdGuard&) = delete;

    FdGuard(FdGuard&& other) noexcept
        : fd_(other.release()) {}

    FdGuard& operator=(FdGuard&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.release();
        }
        return *this;
    }

    int get() const noexcept {
        return fd_;
    }

    bool valid() const noexcept {
        return fd_ >= 0;
    }

    explicit operator bool() const noexcept {
        return valid();
    }

    int release() noexcept {
        int result = fd_;
        fd_ = -1;
        return result;
    }

    void reset(int new_fd = -1) noexcept {
        if (fd_ == new_fd) {
            return;
        }

        if (fd_ >= 0) {
            (void)::close(fd_);
        }

        fd_ = new_fd;
    }

private:
    int fd_ = -1;
};
