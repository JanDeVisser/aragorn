/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <LibCore/JSON.h>

#include <App/App.h>
#include <App/Buffer.h>
#include <App/Theme.h>

namespace Aragorn {

namespace fs = std::filesystem;

enum AppStateItem {
    ASMonitor = 0,
    ASCount,
};

struct AppState {
    int state[ASCount] = { 0 };

    AppState()
    {
        memset(&state, 0, sizeof(state));
    }

    void              read();
    void              write();
    [[nodiscard]] int monitor() const
    {
        return state[ASMonitor];
    }

    void monitor(int the_monitor)
    {
        state[ASMonitor] = the_monitor;
        write();
    }
};

struct SettingsError {
    explicit SettingsError(std::string_view const &e)
        : error(e)
    {
    }

    std::string error;
};

class AragornError {
public:
    template<class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template<typename T>
    explicit AragornError(T const &e)
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

using EError = Error<AragornError>;

using pProject = std::shared_ptr<class Project>;
using pAragorn = std::shared_ptr<struct Aragorn>;

class Project : public Widget {
public:
    struct CMake {
        std::string cmakelists { "CMakeLists.txt" };
        std::string build_dir { "build" };
    };

    std::string project_dir {};
    StringList  source_dirs {};
    CMake       cmake {};

    explicit Project(pWidget const &parent)
        : Widget(parent)
    {
    }

    static Result<pProject, AragornError> open(pAragorn const &aragorn, std::string_view const &dir);
    void                                  close(pAragorn const &aragorn) const;
};

struct Aragorn : public App {
    AppState             app_state {};
    fs::path             system_config_dir {};
    fs::path             user_config_dir {};
    std::vector<pBuffer> buffers {};
    FT_Library           ft_library {};
    JSONValue            settings;
    pProject             project;
    StringList           font_dirs;
    Texture2D            tab_char;
    Texture2D            eol_char;
    float                line_height { 1.5 };
    std::vector<int>     guides {};

    Aragorn();
    static pAragorn the();
    static void     set_message(std::string_view const &text);

    void            initialize() override;
    bool            query_close() override;
    void            on_start() override;
    void            on_resize() override;
    void            process_input() override;
    void            on_terminate() override;
    pBuffer         new_buffer();
    Result<pBuffer> open_buffer(std::string_view const &file);
    void            close_buffer(int buffer_num);
    EError          read_settings();
    EError          load_theme(std::string_view const &name);
    void            load_font();
    StringList      get_font_dirs();
    EError          open_dir(std::string_view const &dir);
    void            terminate();
    pMode           get_mode_for_buffer(pBuffer const &buffer);

    pBuffer const &buffer(int buffer_num)
    {
        assert(buffer_num >= 0 && buffer_num < buffers.size());
        return buffers[buffer_num];
    }

    Theme &theme() { return m_theme; }

private:
    Theme m_theme;
};

}
