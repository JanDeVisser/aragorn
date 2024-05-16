/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <LibCore/JSON.h>

#include <App/App.h>
#include <App/Buffer.h>

namespace Eddy {

enum class AppStateItem {
    Monitor = 0,
    Count,
};

struct AppState {
    int state[static_cast<int>(AppStateItem::Count)] = { 0 };

    AppState()
    {
        memset(&state, 0, sizeof(state));
    }

    void read();
    void write();
};

struct SettingsError {
    explicit SettingsError(std::string_view const &e)
        : error(e)
    {
    }

    std::string error;
};

class EddyError {
public:
    template<class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template<typename T>
    explicit EddyError(T const &e)
        : error(e)
    {
    }

    std::variant<LibCError, JSONError, SettingsError> error;

    [[nodiscard]] std::string to_string() const
    {
        return std::visit(
            overloaded {
                [](LibCError const &e) { return e.to_string(); },
                [](JSONError const &e) { return e.to_string(); },
                [](SettingsError const &e) { return e.error; },
            },
            error);
    }
};

using EError = Error<EddyError>;

using pProject = std::shared_ptr<class Project>;
using pEddy = std::shared_ptr<struct Eddy>;

class Project : public Widget {
public:
    struct CMake {
        std::string cmakelists { "CMakeLists.txt" };
        std::string build_dir { "build" };
    };

    Project() = default;
    std::string project_dir {};
    StringList  source_dirs {};
    CMake       cmake {};

    static Result<pProject, EddyError> open(pEddy const &eddy, std::string_view const &dir);
    void                               close(pEddy const &eddy) const;
};

struct Eddy : public App {
    AppState             app_state {};
    std::vector<pBuffer> buffers {};
    FT_Library           ft_library {};
    JSONValue            settings;
    pProject             project;
    StringList           font_dirs;

    Eddy();
    static pEddy the();
    static void  set_message(std::string_view const &text);

    void            initialize() override;
    pBuffer         new_buffer();
    Result<pBuffer> open_buffer(std::string_view const &file);
    void            close_buffer(int buffer_num);
    EError          read_settings();
    void            load_font();
    StringList      get_font_dirs();
    EError          open_dir(std::string_view const &dir);

    pBuffer const &buffer(int buffer_num)
    {
        assert(buffer_num >= 0 && buffer_num < buffers.size());
        return buffers[buffer_num];
    }
};

}
