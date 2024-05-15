/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>
#include <pwd.h>
#include <system_error>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <LibCore/IO.h>
#include <config.h>

#include <App/Eddy.h>

namespace Eddy {

namespace fs = std::filesystem;

void AppState::read()
{
    struct passwd *pw = getpwuid(getuid());
    struct stat    sb;
    char const    *eddy_fname = TextFormat("%s/.eddy", pw->pw_dir);
    if (stat(eddy_fname, &sb) != 0) {
        mkdir(eddy_fname, 0700);
    }

    char const *state_fname = TextFormat("%s/.eddy/state", pw->pw_dir);
    if (stat(state_fname, &sb) == 0) {
        int state_fd = open(state_fname, O_RDONLY);
        ::read(state_fd, this, sizeof(AppState));
        close(state_fd);
    } else {
        write();
    }
}

void AppState::write()
{
    struct passwd *pw = getpwuid(getuid());
    char const    *state_fname = TextFormat("%s/.eddy/state", pw->pw_dir);
    int            state_fd = open(state_fname, O_RDWR | O_CREAT | O_TRUNC, 0600);
    ::write(state_fd, this, sizeof(AppState));
    close(state_fd);
}

pEddy Eddy::the()
{
    return s_the;
}

Eddy::Eddy()
    : App()
{
    assert(s_the == nullptr);
    app_state.read();
    monitor = app_state.state[static_cast<int>(AppStateItem::Monitor)];
    if (!arguments.empty()) {
        project_dir = arguments.front();
        arguments.pop_front();
    }

    if (FT_Init_FreeType(&ft_library)) {
        fatal("Could not initialize freetype");
    }
    s_the = std::dynamic_pointer_cast<Eddy>(shared_from_this());
}

pBuffer Eddy::new_buffer()
{
    for (auto const &buffer : buffers) {
        if (buffer->name.empty() && buffer->text.empty()) {
            buffer->build_indices();
            return buffer;
        }
    }
    pBuffer b = Buffer::new_buffer();
    buffers.push_back(b);
    return b;
}

Result<pBuffer> Eddy::open_buffer(std::string_view const &file)
{
    for (auto const &buffer : buffers) {
        if (buffer->name == file) {
            buffer->build_indices();
            return buffer;
        }
    }
    pBuffer b = TRY_EVAL(Buffer::open(file));
    buffers.push_back(b);
    return b;
}

EError Eddy::read_settings()
{
    if (!settings.is_null()) {
        return {};
    }
    settings = JSONValue::object();
    settings["appearance"] = JSONValue::object();

    auto merge_settings = [this](fs::path const& dir) -> EError {
        fs::create_directory(dir);
        auto settings_file = dir / "settings.json";
        if (fs::exists(settings_file)) {
            auto json_text_maybe = read_file_by_name(settings_file.string());
            if (json_text_maybe.is_error()) {
                return EddyError { json_text_maybe.error() };
            }
            auto json_maybe = JSONValue::deserialize(json_text_maybe.value());
            if (json_maybe.is_error()) {
                return EddyError { json_maybe.error() };
            }
            settings.merge(json_maybe.value());
        }
        return {};
    };

    if (auto const& e = merge_settings(EDDY_DATADIR); e.is_error()) {
        return e;
    }
    struct passwd *pw = getpwuid(getuid());
    if (auto const& e = merge_settings(fs::path { pw->pw_dir} / ".eddy"); e.is_error()) {
        return e;
    }
    if (auto const& e = merge_settings(".eddy"); e.is_error()) {
        return e;
    }
#ifdef THEME
    JSONValue appearance = json_get_default(&eddy->settings, "appearance", json_object());
    JSONValue theme_name = json_get_default(&appearance, "theme", json_string(SV("darcula", 7)));
    assert(theme_name.type == JSON_TYPE_STRING);
    eddy_load_theme(eddy, theme_name.string);
#endif
    return {};
}

StringList Eddy::get_font_dirs()
{
    static StringList s_font_dirs {};
    if (s_font_dirs.empty()) {
        s_font_dirs.push_back(fs::canonical(EDDY_DATADIR "/fonts"));

        auto& appearance = settings["appearance"];
        auto& directories = appearance["font_directories"];

        struct passwd *pw = getpwuid(getuid());
        auto append_dir = [pw](std::string_view const &dir) -> void {
            std::string d { dir };
            replace_all(d, "${HOME}", pw->pw_dir);
            replace_all(d, "${EDDY_DATADIR}", EDDY_DATADIR);
            if (std::find(s_font_dirs.begin(), s_font_dirs.end(), d) == s_font_dirs.end()) {
                s_font_dirs.push_back(fs::canonical(d));
            }
        };

        if (directories.is_string()) {
            append_dir(directories.to_string());
        } else if (directories.is_array()) {
            StringList dirs {};
            if (auto const e = decode_value(directories, dirs); !e.is_error()) {
                for (auto const& dir : dirs) {
                    append_dir(dir);
                }
            }
        }
    }
    return s_font_dirs;
}

void Eddy::load_font()
{
    auto      appearance = settings["appearance"];
    std::string default_font { "VictorMono-Medium.ttf" };
    auto font_name = default_font;
    auto const font_maybe = appearance.try_get<std::string>("font");
    if (font_maybe.has_value()) {
        font_name = font_maybe.value();
    }
    int font_size = 20;
    auto const font_size_maybe = appearance.try_get<int>("font_size");
    if (font_size_maybe.has_value()) {
        font_size = font_size_maybe.value();
    }

    StringList font_dirs = get_font_dirs();
    auto find_font = [this, &font_dirs, font_size](auto const& font) -> bool {
        for (auto const& dir : font_dirs) {
            auto const path = std::format("{}/{}", dir, font);
            if (fs::exists(path) && !fs::is_directory(path)) {
                set_font(path, font_size);
                return true;
            }
        }
        return false;
    };

    if (!find_font(font_name) && (font_name != default_font)) {
        assert(find_font(default_font));
    }
}

EError Eddy::open_dir(std::string_view const& dir)
{
    assert(buffers.empty());
    auto project_maybe = Project::open(self<Eddy>(), dir);
    if (project_maybe.is_error()) {
        return project_maybe.error();
    }
    return {};
}

pEddy Eddy::s_the { nullptr };

}

int main(int argc, char const **argv)
{
    auto eddy = Eddy::App::create<Eddy::Eddy>(argc, argv);
    eddy->start();
    return 0;
}
