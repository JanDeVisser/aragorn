/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <filesystem>
#include <pwd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

#include <LibCore/IO.h>
#include <config.h>

#include <App/Eddy.h>
#include <App/Editor.h>
#include <App/MiniBuffer.h>
#include <App/Modal.h>
#include <App/StatusBar.h>

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
    return std::dynamic_pointer_cast<Eddy>(App::the());
}

Eddy::Eddy()
    : App()
{
    app_state.read();
    monitor = app_state.monitor();

    if (FT_Init_FreeType(&ft_library)) {
        fatal("Could not initialize freetype");
    }
}

bool Eddy::query_close()
{
    submit("eddy-quit", JSONValue {});
    return false;
}

void Eddy::on_start()
{
    monitor = GetCurrentMonitor();
}

void Eddy::process_input()
{
    if (monitor != app_state.monitor()) {
        app_state.monitor(monitor);
    }
    App::process_input();
}

void Eddy::on_terminate()
{
    if (font) {
        UnloadFont(*font);
    }
}

void Eddy::terminate()
{
    project->close(self<Eddy>());
    quit = true;
}

void cmd_force_quit(pEddy const &eddy, JSONValue const &)
{
    eddy->terminate();
}

void cmd_quit(pEddy const &eddy, JSONValue const &)
{
    if (!eddy->modals.empty()) {
        // User probably clicked the close window button twice
        return;
    }
    bool has_modified_buffers = false;
    for (auto const &buffer : eddy->buffers) {
        if (buffer->saved_version < buffer->version) {
            has_modified_buffers = true;
            break;
        }
    }

    int         selection = 0;
    auto const *prompt = "Are you sure you want to quit?";
    if (has_modified_buffers) {
        selection = 1;
        prompt = "There are modified files. Are you sure you want to quit?";
    }

    auto are_you_sure = [](pEddy const &eddy, QueryOption selection) {
        if (selection == QueryOptionYes) {
            eddy->terminate();
        }
    };
    query_box(eddy, prompt, are_you_sure, QueryOptionYesNo);
}

void cmd_run_command(pEddy const &eddy, JSONValue const &)
{
    struct Commands : public ListBox<Widget::WidgetCommand> {
        pEddy const &eddy;
        explicit     Commands(pEddy const &eddy)
            : ListBox("Select command")
            , eddy(eddy)
        {
            auto push_commands = [this](auto &w) -> void {
                for (auto &[name, command] : w->commands) {
                    entries.emplace_back(name, command);
                }
            };
            for (pWidget &w = eddy->focus; w; w = w->parent) {
                push_commands(w);
                if (w->delegate) {
                    push_commands(w->delegate);
                }
            }
        }

        void submit() override
        {
            auto const &cmd = entries[selection].payload;
            cmd.owner->submit(cmd.command, JSONValue {});
            eddy->set_message(std::format("Selected command '{}'", cmd.command));
        }
    };
    Commands commands { eddy };
    commands.show();
}

void Eddy::initialize()
{
    auto res = read_settings();
    if (res.is_error()) {
        fatal("Error reading settings: {}", res.error().to_string());
    }
    load_font();

    auto editor_pane = Widget::make<Layout>(ContainerOrientation::Horizontal);
    editor_pane->parent = self();
    editor_pane->policy = SizePolicy::Stretch;
    auto editor = editor_pane->add_widget<Editor>();
    editor_pane->insert_widget<Gutter>(0, editor);
    auto main_area = Widget::make<Layout>(ContainerOrientation::Vertical);
    main_area->policy = SizePolicy::Stretch;
    main_area->append(editor_pane);
    main_area->add_widget<StatusBar>();
    main_area->add_widget<MiniBuffer>();
    append(main_area);

    add_command<Eddy>("eddy-force-quit", cmd_force_quit)
        .bind(KeyCombo { KEY_Q, KModControl | KModShift });
    add_command<Eddy>("eddy-quit", cmd_quit)
        .bind(KeyCombo { KEY_Q, KModControl });
    add_command<Eddy>("eddy-run-command", cmd_run_command)
        .bind(KeyCombo { KEY_P, KModSuper | KModShift });

    std::string project_dir { "." };
    if (!arguments.empty()) {
        auto project_dir_maybe = arguments.front();
        if (fs::is_directory(project_dir_maybe)) {
            project_dir = project_dir_maybe;
            arguments.pop_front();
        }
    }
    auto project_maybe = Project::open(self<Eddy>(), project_dir);
    if (project_maybe.is_error()) {
        fatal("Could not open project directory '{}': {}", project_dir, project_maybe.error().to_string());
    }
    project = project_maybe.value();
    while (!arguments.empty()) {
        auto &fname = arguments.front();
        arguments.pop_front();
        auto open_res = open_buffer(fname);
        if (open_res.is_error()) {
            set_message(std::format("Could not open '{}': {}", fname, open_res.error().to_string()));
        }
    }
    if (buffers.empty()) {
        new_buffer();
    }
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
    auto const &editor = find_by_class<Editor>();
    editor->select_buffer(b);
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
    auto const &editor = find_by_class<Editor>();
    editor->select_buffer(b);
    return b;
}

void Eddy::close_buffer(int buffer_num)
{
    assert(buffer_num >= 0 && buffer_num < buffers.size());
    pBuffer buffer = buffers[buffer_num];
    buffer->close();
    buffers.erase(buffers.begin() + buffer_num);
}

EError Eddy::read_settings()
{
    if (!settings.is_null()) {
        return {};
    }
    settings = JSONValue::object();
    settings["appearance"] = JSONValue::object();

    auto merge_settings = [this](fs::path const &dir, std::string const& file = "settings.json") -> EError {
        create_directory(dir);
        auto settings_file = dir / file;
        if (exists(settings_file)) {
            auto json_maybe = JSONValue::read_file(settings_file.string());
            if (json_maybe.is_error()) {
                switch (json_maybe.error().index()) {
                case 0:
                    return EddyError { std::get<LibCError>(json_maybe.error()) };
                case 1:
                    return EddyError { std::get<JSONError>(json_maybe.error()) };
                default:
                    UNREACHABLE();
                }
            }
            settings.merge(json_maybe.value());
        }
        return {};
    };

    if (auto const &e = merge_settings(EDDY_DATADIR, EDDY_SYSTEM ".json"); e.is_error()) {
        return e;
    }
    if (auto const &e = merge_settings(EDDY_DATADIR); e.is_error()) {
        return e;
    }
    struct passwd *pw = getpwuid(getuid());
    if (auto const &e = merge_settings(fs::path { pw->pw_dir } / ".eddy"); e.is_error()) {
        return e;
    }
    if (auto const &e = merge_settings(".eddy"); e.is_error()) {
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
    if (font_dirs.empty()) {
        auto &appearance = settings["appearance"];
        auto &directories = appearance["font_directories"];

        struct passwd *pw = getpwuid(getuid());
        auto           append_dir = [pw, this](std::string_view const &dir) -> void {
            std::string d { dir };
            replace_all(d, "${HOME}", pw->pw_dir);
            replace_all(d, "${EDDY_DATADIR}", EDDY_DATADIR);
            if (std::find(font_dirs.begin(), font_dirs.end(), d) == font_dirs.end()) {
                auto const base_dir { d };
                if (!fs::is_directory(base_dir)) {
                    return;
                }
                font_dirs.push_back(base_dir);
                for (auto const& dir_entry : fs::recursive_directory_iterator(base_dir)) {
                    if (dir_entry.is_directory()) {
                        font_dirs.push_back(base_dir);
                    }
                }
            }
        };

        if (directories.is_string()) {
            append_dir(directories.to_string());
        } else if (directories.is_array()) {
            StringList dirs {};
            if (auto const e = decode_value(directories, dirs); !e.is_error()) {
                for (auto const &dir : dirs) {
                    append_dir(dir);
                }
            }
        }
    }
    return font_dirs;
}

void Eddy::load_font()
{
    auto        appearance = settings["appearance"];
    std::string default_font { "VictorMono-Medium.ttf" };
    std::string font_name { default_font };
    auto const  font_maybe = appearance.try_get<std::string>("font");
    if (font_maybe.has_value()) {
        font_name = font_maybe.value();
    }
    int        font_size = 20;
    auto const font_size_maybe = appearance.try_get<int>("font_size");
    if (font_size_maybe.has_value()) {
        font_size = font_size_maybe.value();
    }

    StringList font_dirs = get_font_dirs();
    auto       find_font = [this, &font_dirs, font_size](auto const &font) -> bool {
        return std::any_of(font_dirs.begin(), font_dirs.end(), [this, font, font_size](auto const &dir) -> bool {
            if (fs::exists(dir) && fs::is_directory(dir)) {
                if (auto const path = fs::path {dir} / font; fs::exists(path) && !fs::is_directory(path)) {
                   set_font(path.string(), font_size);
                   return true;
               }
            }
            return false;
        });
    };

    if (!find_font(font_name) && (font_name != default_font)) {
        assert(find_font(default_font));
    }
}

EError Eddy::open_dir(std::string_view const &dir)
{
    assert(buffers.empty());
    auto project_maybe = Project::open(self<Eddy>(), dir);
    if (project_maybe.is_error()) {
        return project_maybe.error();
    }
    return {};
}

void Eddy::set_message(std::string_view const &text)
{
    MiniBuffer::set_message(text);
}

}

int main(int argc, char const **argv)
{
    auto eddy = Eddy::App::create<Eddy::Eddy>(argc, argv);
    eddy->start();
    return 0;
}
