/*
 * Copyright (c) 2025, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <locale>

#include <LibCore/Result.h>

namespace LibCore {

Result<std::string> to_utf8(std::wstring_view const &s);
Result<std::wstring> to_wstring(std::string_view const &s);
Result<ssize_t> write_utf8(std::ofstream &os, std::wstring_view const &contents);
Result<std::wstring> read_utf8(std::ifstream &is);

}
