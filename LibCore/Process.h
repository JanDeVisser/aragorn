/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <LibCore/Error.h>
#include <LibCore/Pipe.h>
#include <LibCore/Result.h>
#include <LibCore/StringUtil.h>

namespace LibCore {

void sigchld(int);

template<class T = void *>
class Process {
public:
    Process(std::string_view const &cmd) noexcept
        : m_command(cmd)
    {
    }

    template<typename... Args>
    Process(std::string_view const &cmd, Args &&...args)
        : Process(cmd)
    {
        StringList cmd_args = {};
        set_arguments(cmd_args, std::forward<Args>(args)...);
    }

    ~Process()
    {
    }

    pid_t pid() const { return m_pid; }

    Result<int, LibCError> start(T const &ctx)
    {
        signal(SIGCHLD, sigchld);
        size_t sz = m_arguments.size();
        size_t bufsz = m_command.length() + 1;
        for (size_t ix = 0u; ix < sz; ++ix) {
            bufsz += m_arguments[ix].length() + 1;
        }
        char        buf[bufsz];
        char const *argv[sz + 2];
        argv[0] = m_command.c_str();
        strcpy(buf, argv[0]);
        char *bufptr = buf + m_command.length() + 1;
        for (size_t ix = 0u; ix < sz; ++ix) {
            argv[ix + 1] = m_arguments[ix].c_str();
            bufptr = bufptr + m_arguments[ix].length() + 1;
        }
        argv[sz + 1] = NULL;
        trace(PROCESS, "[CMD] {} {}", m_command, join(m_arguments, " "));

        // signal(SIGCHLD, SIG_IGN);
        if (auto err = m_in.initialize(); err) {
            return err;
        }
        if (auto err = m_out.initialize(); err) {
            return err;
        }
        m_out.context(ctx);
        if (auto err = m_err.initialize(); err) {
            return err;
        }
        m_err.context(ctx);

        m_pid = fork();
        if (m_pid == -1) {
            return LibCError(errno);
        }
        if (m_pid == 0) {
            m_in.connect_child(STDIN_FILENO);

            auto connect_stream = [this](ReadPipe<T> &pipe, int fileno, std::string const &redirect) {
                if (redirect.empty()) {
                    pipe.connect_child(fileno);
                } else {
                    int fd = open(redirect.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
                    assert_msg(fd, "Could not open stdout stream '%s' for '%s': %s",
                        redirect, m_command, strerror(errno));
                    while (dup2(fd, fileno) == -1 && (errno == EINTR)) { }
                }
            };
            connect_stream(m_out, STDOUT_FILENO, m_stdout_file);
            connect_stream(m_err, STDERR_FILENO, m_stderr_file);
            ::execvp(argv[0], (char *const *) argv);
            fatal("execvp({}) failed: {}", m_command, LibCError(errno).description);
        }
        m_in.connect_parent();
        m_out.connect_parent();
        m_err.connect_parent();
        return m_pid;
    }

    Result<int, LibCError> background()
    {
        return start();
    }

    Result<int, LibCError> wait()
    {
        if (m_pid == 0) {
            return { 0 };
        }
        int exit_code;
        if (waitpid(m_pid, &exit_code, 0) == -1 && errno != ECHILD && errno != EINTR) {
            return LibCError();
        }
        m_pid = 0;
        m_in.close();
        if (!WIFEXITED(exit_code)) {
            LibCError("Child program {} crashed due to signal {}", m_command, WTERMSIG(exit_code));
        }
        return { WEXITSTATUS(exit_code) };
    }

    Result<int, LibCError> start()
    {
        T ctx = {};
        return start(ctx);
    }

    Result<int, LibCError> execute()
    {
        if (auto err = start(); err) {
            return err;
        }
        return wait();
    }

private:
    void set_arguments(StringList const &cmd_args)
    {
        m_arguments = cmd_args;
    }

    template<typename... Args>
    void set_arguments(StringList &cmd_args, std::string_view const &arg, Args &&...args)
    {
        cmd_args.emplace_back(arg);
        set_arguments(cmd_args, std::forward<Args>(args)...);
    }

    pid_t                    m_pid;
    std::string              m_command;
    std::vector<std::string> m_arguments;
    WritePipe                m_in;
    ReadPipe<T>              m_out;
    ReadPipe<T>              m_err;
    std::string              m_stdout_file;
    std::string              m_stderr_file;
};

}
