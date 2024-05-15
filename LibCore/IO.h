/*
 * Copyright (c) 2023, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <netinet/in.h>
#include <string>

#include <LibCore/Result.h>

namespace LibCore {

using socket_t = std::shared_ptr<struct Socket>;

struct Socket {
    int         fd { 0 };
    std::string buffer {};

    Result<socket_t>    accept();
    CError              make_nonblocking();
    Result<std::string> read(size_t count);
    Result<std::string> readln();
    Result<size_t>      write(std::string_view const &buf, size_t num);
    Result<size_t>      writeln(std::string_view const &buf);
    CError              close();

    Socket(int fd);
    static Result<socket_t> listen(std::string_view const &unix_socket_name);
    static Result<socket_t> listen(std::string_view const &ip_address, int port);
    static Result<socket_t> connect(std::string_view const &unix_socket_name);
    static Result<socket_t> connect(std::string_view const &ip_address, int port);

private:
    Result<size_t> read_available_bytes();
    Result<size_t> fill_buffer();
};

Result<struct sockaddr_in> tcpip_address_resolve(std::string_view const &ip_address);
CError                     fd_make_nonblocking(int fd);
Result<std::string>        read_file_by_name(std::string_view const &file_name);
Result<std::string>        read_file_at(int dir_fd, std::string_view const &file_name);
Result<std::string>        read_file(int fd);
Result<ssize_t>             write_file_by_name(std::string_view const &file_name, std::string_view const &contents);
Result<ssize_t>             write_file_at(int dir_fd, std::string_view const &file_name, std::string_view const &contents);
Result<ssize_t>             write_file(int fd, std::string_view const &contents);

}
