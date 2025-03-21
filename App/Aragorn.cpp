/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <pwd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

#include <config.h>

#include <App/Aragorn.h>
#include <App/CMode.h>
#include <App/Editor.h>
#include <App/LexerMode.h>
#include <App/MiniBuffer.h>
#include <App/Modal.h>
#include <App/StatusBar.h>
#include <LibCore/IO.h>

namespace Aragorn {

namespace fs = std::filesystem;

void AppState::read()
{
    struct passwd *pw = getpwuid(getuid());
    struct stat    sb;
    char const    *aragorn_fname = TextFormat("%s/.aragorn", pw->pw_dir);
    if (stat(aragorn_fname, &sb) != 0) {
        mkdir(aragorn_fname, 0700);
    }

    char const *state_fname = TextFormat("%s/.aragorn/state", pw->pw_dir);
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
    char const    *state_fname = TextFormat("%s/.aragorn/state", pw->pw_dir);
    int            state_fd = open(state_fname, O_RDWR | O_CREAT | O_TRUNC, 0600);
    ::write(state_fd, this, sizeof(AppState));
    close(state_fd);
}

pAragorn Aragorn::the()
{
    return std::dynamic_pointer_cast<Aragorn>(App::the());
}

Aragorn::Aragorn()
    : App()
{
    app_state.read();
    monitor = app_state.monitor();
    passwd *pw = getpwuid(getuid());
    user_config_dir = fs::path { pw->pw_dir } / ".aragorn";
    system_config_dir = fs::path { ARAGORN_DATADIR };

    if (FT_Init_FreeType(&ft_library)) {
        fatal("Could not initialize freetype");
    }
}

bool Aragorn::query_close()
{
    submit("aragorn-quit", JSONValue {});
    return false;
}

void Aragorn::on_start()
{
    monitor = GetCurrentMonitor();
}

void Aragorn::on_resize()
{
    App::on_resize();

    auto const &appearance = settings.get("appearance").value_or(JSONValue(JSONType::Object));
    MUST(settings.get("line_height").value_or(JSONValue { 1.5 }).convert<float>(line_height));
    auto lines = static_cast<int>(viewport.height) / static_cast<int>(char_size.y * line_height);
    cell.y = viewport.height / static_cast<float>(lines);
    auto cols = static_cast<int>(viewport.width) / static_cast<int>(char_size.x);
    cell.x = viewport.width / static_cast<float>(cols);
}

void Aragorn::process_input()
{
    if (monitor != app_state.monitor()) {
        app_state.monitor(monitor);
    }
    App::process_input();
}

void Aragorn::on_terminate()
{
    if (font) {
        UnloadFont(*font);
    }
}

void Aragorn::terminate()
{
    project->close(self<Aragorn>());
    quit = true;
}

void cmd_force_quit(pAragorn const &aragorn, JSONValue const &)
{
    aragorn->terminate();
}

void cmd_quit(pAragorn const &aragorn, JSONValue const &)
{
    if (!aragorn->modals.empty()) {
        // User probably clicked the close window button twice
        return;
    }
    bool has_modified_buffers = false;
    for (auto const &buffer : aragorn->buffers) {
        if (buffer->saved_version < buffer->version) {
            has_modified_buffers = true;
            break;
        }
    }

    auto const *prompt = "Are you sure you want to quit?";
    if (has_modified_buffers) {
        prompt = "There are modified files. Are you sure you want to quit?";
    }

    auto are_you_sure = [](pAragorn const &aragorn, QueryOption selection) {
        if (selection == QueryOptionYes) {
            aragorn->terminate();
        }
    };
    query_box(aragorn, prompt, are_you_sure, QueryOptionYesNo);
}

void cmd_select_font(pAragorn const &aragorn, JSONValue const &)
{
    struct Fonts : public ListBox<fs::directory_entry> {
        explicit Fonts()
            : ListBox("Select font")
        {
            auto push_fonts = [this](std::string const &dir) -> void {
                for (auto const &entry : fs::directory_iterator(dir)) {
                    FT_Face face;
                    if (FT_New_Face(Aragorn::the()->ft_library, entry.path().c_str(), 0, &face) != 0) {
                        continue;
                    }
                    if (!FT_IS_FIXED_WIDTH(face)) {
                        continue;
                    }
                    if (FT_Get_Char_Index(face, 'A') == 0) {
                        continue;
                    }
                    entries.emplace_back(entry.path().stem().string(), entry);
                }
            };
            for (auto const &dir : Aragorn::the()->get_font_dirs()) {
                push_fonts(dir);
            }
        }

        void submit() override
        {
            auto const &font = entries[selection].payload;
            Aragorn::the()->set_font(font.path().string(), Aragorn::the()->font_size);
            Aragorn::set_message(std::format("Selected fonts '{}'", font.path().string()));
        }
    };
    auto fonts = Widget::make<Fonts>();
    fonts->show();
}

void cmd_select_theme(pAragorn const &aragorn, JSONValue const &)
{
    struct Themes : public ListBox<fs::directory_entry> {
        explicit Themes()
            : ListBox("Select Theme")
        {
            auto push_themes = [this](fs::path const &dir) -> void {
                if (exists(dir) && is_directory(dir)) {
                    for (auto const &entry : fs::directory_iterator(dir)) {
                        if (entry.path().extension().string() == ".json") {
                            entries.emplace_back(entry.path().stem().string(), entry);
                        }
                    }
                }
            };
            auto aragorn = Aragorn::the();
            push_themes(aragorn->system_config_dir / "themes");
            push_themes(aragorn->user_config_dir / "themes");
        }

        void submit() override
        {
            auto const &file = entries[selection].payload;
            if (auto e = Aragorn::the()->load_theme(file.path().stem().string()); e.is_error()) {
                Aragorn::set_message(std::format("Error loading theme '{}': {}", entries[selection].text, e.error().to_string()));
            }
            Aragorn::set_message(std::format("Loaded theme '{}'", entries[selection].text));
        }
    };
    auto themes = Widget::make<Themes>();
    themes->show();
}

void cmd_run_command(pAragorn const &aragorn, JSONValue const &)
{
    struct Commands : public ListBox<Widget::WidgetCommand> {
        explicit Commands()
            : ListBox("Select command")
        {
            auto push_commands = [this](auto &w) -> void {
                for (auto &[name, command] : w->commands) {
                    entries.emplace_back(name, command);
                }
            };
            for (pWidget &w = Aragorn::the()->focus; w; w = w->parent) {
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
            Aragorn::set_message(std::format("Selected command '{}'", cmd.command));
        }
    };
    auto commands = Widget::make<Commands>();
    commands->show();
}

void cmd_lsp_log(pAragorn const &aragorn, JSONValue const &)
{
}

void Aragorn::initialize()
{
    auto res = read_settings();
    if (res.is_error()) {
        fatal("Error reading settings: {}", res.error().to_string());
    }
    load_font();

    std::string project_dir { "." };
    if (!arguments.empty()) {
        auto project_dir_maybe = arguments.front();
        if (fs::is_directory(project_dir_maybe)) {
            project_dir = project_dir_maybe;
            arguments.pop_front();
        }
    }

    // FIXME split up Project::open into static Project::create and instance
    // Project::open.
    auto project_maybe = Project::open(self<Aragorn>(), project_dir);
    if (project_maybe.is_error()) {
        fatal("Could not open project directory '{}': {}", project_dir, project_maybe.error().to_string());
    }
    assert(project == project_maybe.value());
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

    auto editor_pane = Widget::make<Layout>(self(), ContainerOrientation::Horizontal);
    editor_pane->policy = SizePolicy::Stretch;
    auto   editor = editor_pane->add_widget<Editor>();
    size_t ix = 0;
    while (ix < buffers.size() && !buffers[ix]->name.empty() && buffers[ix]->name[0] == '*')
        ++ix;
    editor->select_buffer(buffers[ix]);
    editor_pane->insert_widget<Gutter>(0, editor);
    auto main_area = Widget::make<Layout>(Aragorn::the(), ContainerOrientation::Vertical);
    main_area->policy = SizePolicy::Stretch;
    main_area->append(editor_pane);
    main_area->add_widget<StatusBar>();
    main_area->add_widget<MiniBuffer>();
    append(main_area);

    add_command<Aragorn>("aragorn-force-quit", cmd_force_quit)
        .bind(KeyCombo { KEY_Q, KModControl | KModShift });
    add_command<Aragorn>("aragorn-quit", cmd_quit)
        .bind(KeyCombo { KEY_Q, KModControl });
    add_command<Aragorn>("aragorn-run-command", cmd_run_command)
        .bind(KeyCombo { KEY_P, KModSuper | KModShift });
    add_command<Aragorn>("aragorn-select-font", cmd_select_font);
    add_command<Aragorn>("aragorn-select-theme", cmd_select_theme)
        .bind(KeyCombo { KEY_GRAVE, KModControl });

    auto tab_img = GenImageColor(cell.x * 2, cell.y, BLANK);
    ImageDrawLine(
        &tab_img,
        0, static_cast<int>(cell.y / 2),
        static_cast<int>(1.5 * cell.x), static_cast<int>(cell.y / 2),
        Theme::the().fg());
    ImageDrawLine(
        &tab_img,
        static_cast<int>(1.5 * cell.x), static_cast<int>(cell.y / 3),
        static_cast<int>(1.5 * cell.x), 2 * static_cast<int>(cell.y / 3),
        Theme::the().fg());
    ImageDrawLine(
        &tab_img,
        static_cast<int>(1.5 * cell.x), static_cast<int>(cell.y / 2),
        static_cast<int>(cell.x), static_cast<int>(cell.y / 3),
        Theme::the().fg());
    ImageDrawLine(
        &tab_img,
        static_cast<int>(1.5 * cell.x), static_cast<int>(cell.y / 2),
        static_cast<int>(cell.x), static_cast<int>(2 * cell.y / 3),
        Theme::the().fg());
    tab_char = LoadTextureFromImage(tab_img);
}

pBuffer Aragorn::new_buffer()
{
    for (auto const &buffer : buffers) {
        if (buffer->name.empty() && buffer->empty()) {
            buffer->lex();
            return buffer;
        }
    }
    pBuffer b = Buffer::new_buffer();
    buffers.push_back(b);
    return b;
}

pBuffer Aragorn::create_system_buffer(std::string_view name)
{
    pBuffer b = Buffer::new_buffer();
    buffers.push_back(b);
    b->name = std::format("*{}", name);
    b->read_only = true;
    return b;
}

Result<pBuffer> Aragorn::open_buffer(std::string_view const &file)
{
    for (auto const &buffer : buffers) {
        if (buffer->name == file) {
            buffer->lex();
            return buffer;
        }
    }
    pBuffer b = TRY_EVAL(Buffer::open(file));
    buffers.push_back(b);
    return b;
}

void Aragorn::close_buffer(int buffer_num)
{
    assert(buffer_num >= 0 && buffer_num < buffers.size());
    pBuffer buffer = buffers[buffer_num];
    buffer->close();
    buffers.erase(buffers.begin() + buffer_num);
}

EError Aragorn::read_settings()
{
    if (!settings.is_null()) {
        return {};
    }
    settings = JSONValue::object();
    settings["appearance"] = JSONValue::object();

    auto merge_settings = [this](fs::path const &dir, std::string const &file = "settings.json") -> EError {
        create_directory(dir);
        auto settings_file = dir / file;
        std::println("--> {}", settings_file.string());
        if (exists(settings_file)) {
            auto json_maybe = JSONValue::read_file(settings_file.string());
            if (json_maybe.is_error()) {
                switch (json_maybe.error().index()) {
                case 0:
                    return AragornError { std::get<LibCError>(json_maybe.error()) };
                case 1:
                    return AragornError { std::get<JSONError>(json_maybe.error()) };
                default:
                    UNREACHABLE();
                }
            }
            settings.merge(json_maybe.value());
        }
        return {};
    };

    if (auto const &e = merge_settings(ARAGORN_DATADIR, ARAGORN_SYSTEM ".json"); e.is_error()) {
        return e;
    }
    if (auto const &e = merge_settings(system_config_dir); e.is_error()) {
        return e;
    }
    if (auto const &e = merge_settings(user_config_dir); e.is_error()) {
        return e;
    }
    if (auto const &e = merge_settings(".aragorn"); e.is_error()) {
        return e;
    }
    auto appearance = settings.get_with_default("appearance");
    ASSERT_JSON_TYPE(appearance, Object);
    std::string theme_name = "darcula";
    if (auto theme_name_value = appearance.get("theme")) {
        theme_name = theme_name_value.value().to_string();
    }
    TRY(load_theme(theme_name));

    auto h = appearance.get("line_height").value_or(JSONValue(1.5));
    if (h.convert<float>(line_height).is_error()) {
        line_height = 1.5f;
    }
    for (auto const &guides_cols = appearance.get("guides").value_or(JSONValue(JSONType::Array)); auto const &guide : guides_cols) {
        int g;
        if (!guide.convert(g).is_error()) {
            guides.emplace_back(g);
        }
    }

    return {};
}

EError Aragorn::load_theme(std::string_view const &name)
{
    auto theme_maybe = Theme::load(name);
    if (theme_maybe.is_error()) {
        return AragornError { theme_maybe.error() };
    }
    m_theme = theme_maybe.value();
    if (!widgets.empty()) {
        resize();
        for (auto const &buffer : buffers) {
            bool is_saved = buffer->version == buffer->saved_version;
            ++buffer->version;
            buffer->lex();
            if (is_saved) {
                buffer->saved_version = buffer->version;
            }
        }
    }
    return {};
}

StringList Aragorn::get_font_dirs()
{
    if (font_dirs.empty()) {
        auto &appearance = settings["appearance"];
        auto &directories = appearance["font_directories"];

        struct passwd *pw = getpwuid(getuid());
        auto           append_dir = [pw, this](std::string_view const &dir) -> void {
            std::string d { dir };
            replace_all(d, "${HOME}", pw->pw_dir);
            replace_all(d, "${ARAGORN_DATADIR}", ARAGORN_DATADIR);
            if (std::find(font_dirs.begin(), font_dirs.end(), d) == font_dirs.end()) {
                if (!fs::is_directory(d)) {
                    return;
                }
                font_dirs.push_back(d);
                for (auto const &dir_entry : fs::recursive_directory_iterator(d)) {
                    if (dir_entry.is_directory()) {
                        font_dirs.push_back(d);
                    }
                }
            }
        };

        if (directories.is_string()) {
            append_dir(directories.to_string());
        } else if (directories.is_array()) {
            std::vector<std::string> dirs;
            if (!directories.convert(dirs).is_error()) {
                for (auto const &dir : dirs) {
                    append_dir(dir);
                }
            }
        }
    }
    return font_dirs;
}

void Aragorn::load_font()
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

    StringList dirs = get_font_dirs();
    auto       find_font = [this, &dirs, font_size](auto const &font) -> bool {
        return std::any_of(dirs.begin(), dirs.end(), [this, font, font_size](auto const &dir) -> bool {
            if (fs::exists(dir) && fs::is_directory(dir)) {
                if (auto const path = fs::path { dir } / font; fs::exists(path) && !fs::is_directory(path)) {
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

EError Aragorn::open_dir(std::string_view const &dir)
{
    assert(buffers.empty());
    auto project_maybe = Project::open(self<Aragorn>(), dir);
    if (project_maybe.is_error()) {
        return project_maybe.error();
    }
    return {};
}

pMode Aragorn::get_mode_for_buffer(pBuffer const &buffer)
{
    auto const      &name = buffer->name;
    std::string_view content_type = "text/plain";
    pMode            ret {};
    if (name.ends_with(".cpp") || name.ends_with(".c") || name.ends_with(".h")) {
        ret = Widget::make<LexerMode<CLexer>>(buffer);
    } else {
        ret = Widget::make<LexerMode<PlainTextLexer>>(buffer);
    }
    return ret;
}

void Aragorn::set_message(std::string_view const &text)
{
    MiniBuffer::set_message(text);
}

}

int main(int argc, char const **argv)
{
    auto aragorn = Aragorn::App::create<Aragorn::Aragorn>(argc, argv);
    aragorn->start();
    return 0;
}
