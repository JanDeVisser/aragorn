/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <condition_variable>
#include <functional>
#include <string>
#include <string_view>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <thread>
#include <unistd.h>

#include <LibCore/Result.h>
#include <LibCore/Logging.h>

namespace LibCore {

constexpr static int PipeEndRead = 0;
constexpr static int PipeEndWrite = 1;

template<typename T = void *>
class ReadPipe {
public:
    using OnPipeRead = std::function<void(ReadPipe<T> &)>;

    ReadPipe() = default;
    ReadPipe(ReadPipe const &) = delete;
    ReadPipe(ReadPipe &&) = delete;
    ~ReadPipe()
    {
        close();
    }

    Error<LibCError> initialize()
    {
        if (pipe(m_pipe) == -1) {
            return LibCError();
        }
        return {};
    }

    Error<LibCError> connect(int fd)
    {
        m_fd = fd;
        if (fcntl(m_fd, F_SETFL, O_NONBLOCK) < 0) {
            return LibCError();
        }

        std::thread thr = std::thread(&ReadPipe::read, this);
        thr.detach();
        return {};
    }

    void connect_parent()
    {
        connect(m_pipe[PipeEndRead]);
        ::close(m_pipe[PipeEndWrite]);
    }

    void connect_child(int fd)
    {
        while ((dup2(m_pipe[PipeEndWrite], fd) == -1) && (errno == EINTR)) { }
        ::close(m_pipe[PipeEndRead]);
        ::close(m_pipe[PipeEndWrite]);
    }

    void close()
    {
        m_condition.notify_all();
        if (m_fd >= 0) {
            ::close(m_fd);
        }
        m_fd = -1;
    }

    bool expect()
    {
        std::unique_lock<std::recursive_mutex> lk(m_mutex);
        m_condition.wait(lk, [this]() {
            return !m_current.empty() || m_fd < 0;
        });
        if (m_fd < 0) {
            return false;
        }
        return true;
    }

    std::string current()
    {
        std::unique_lock<std::recursive_mutex> lk(m_mutex);
        m_condition.wait(lk, [this]() {
            return !m_current.empty() || m_fd < 0;
        });
        if (m_fd < 0) {
            return {};
        }
        std::string ret = std::move(m_current);
        m_current = "";
        return ret;
    }

    T &context()
    {
        return m_context;
    }

    void context(T const &ctx)
    {
        m_context = ctx;
    }

private:
    void read()
    {
        struct pollfd poll_fd = { 0 };
        poll_fd.fd = m_fd;
        poll_fd.events = POLLIN;

        while (true) {
            if (poll(&poll_fd, 1, -1) == -1) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            if (poll_fd.revents & POLLIN) {
                drain();
            }
            if (poll_fd.revents & POLLHUP) {
                break;
            }
        }
        close();
    }

    constexpr static int DRAIN_SIZE = (64 * 1024);

    Error<LibCError> drain()
    {
        char                                   buffer[DRAIN_SIZE];
        std::unique_lock<std::recursive_mutex> lk(m_mutex);
        while (true) {
            ssize_t count = read(m_fd, buffer, sizeof(buffer) - 1);
            if (count >= 0) {
                buffer[count] = 0;
                if (count > 0) {
                    m_current.append(buffer, count);
                    if (count == sizeof(buffer) - 1) {
                        continue;
                    }
                }
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            return LibCError();
        }
        if (m_on_read) {
            m_on_read(pipe);
        }
    }

    int                     m_pipe[2] = { 0 };
    int                     m_fd = { -1 };
    std::string             m_current = {};
    std::recursive_mutex    m_mutex = {};
    std::condition_variable m_condition = {};
    OnPipeRead              m_on_read = { nullptr };
    T                       m_context = {};
    bool                    m_debug = { false };
};

class WritePipe {
public:
    WritePipe() = default;
    WritePipe(WritePipe const &) = delete;
    WritePipe(WritePipe &&from) = delete;

    ~WritePipe()
    {
        close();
    }

    Error<LibCError> initialize()
    {
        if (pipe(m_pipe) == -1) {
            return LibCError();
        }
        return {};
    }

    void close()
    {
        if (m_fd >= 0) {
            ::close(m_fd);
        }
        m_fd = -1;
    }

    void connect_parent()
    {
        m_fd = m_pipe[PipeEndWrite];
        ::close(m_pipe[PipeEndRead]);
    }

    void connect_child(int fd)
    {
        while ((dup2(m_pipe[PipeEndRead], fd) == -1) && (errno == EINTR)) { }
        ::close(m_pipe[PipeEndRead]);
        ::close(m_pipe[PipeEndWrite]);
    }

    Result<size_t> write(std::string_view sv) const
    {
        return write_chars(sv.data(), sv.length());
    }

    Result<size_t> write_chars(char const *buf, size_t num) const
    {
        ssize_t total = { 0 };
        while (total < num) {
            ssize_t count = ::write(m_fd, buf + total, num - total);
            if (count < 0) {
                if (errno != EINTR) {
                    return LibCError();
                }
                continue;
            }
            total += count;
        }
        return { total };
    }

private:
    int m_pipe[2];
    int m_fd;
};

}
