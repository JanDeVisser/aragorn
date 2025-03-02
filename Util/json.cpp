/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

#include <LibCore/IO.h>
#include <LibCore/JSON.h>
#include <LibCore/Options.h>

using namespace LibCore;

int main(int argc, char const **argv)
{
    auto app_args = LibCore::parse_options(argc, argv);
    std::string file {};
    for (auto ix = app_args; ix < argc; ++ix) {
        file = argv[ix];
    }
    if (file.empty()) {
        return 1;
    }
    auto text_maybe = read_file_by_name(file);
    if (text_maybe.is_error()) {
        std::cerr << text_maybe.error().to_string() << std::endl;
        return 1;
    }
    auto json_maybe = JSONValue::deserialize(text_maybe.value());
    if (json_maybe.is_error()) {
        std::cerr << "JSON parse error: " << json_maybe.error().to_string() << std::endl;
        return 1;
    }
    auto res = json_maybe.value().serialize(true);
    std::cout << res << std::endl;
    return 0;
}
