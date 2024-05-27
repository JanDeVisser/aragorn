/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

#include <App/Eddy.h>
#include <App/Editor.h>
#include <App/MiniBuffer.h>
#include <App/Modal.h>

namespace Eddy {

Result<int> Editor::open(std::string_view const &file)
{
    auto buffer = TRY_EVAL(Eddy::the()->open_buffer(file));
    select_buffer(buffer);
    return current_view_ix;
}

void Editor::new_buffer()
{
    auto const &buffer = Eddy::the()->new_buffer();
    select_buffer(buffer);
}

void Editor::select_view(int view_ix)
{
    assert(view_ix >= 0 && view_ix < views.size());
    if (current_view_ix >= 0 && current_view_ix < views.size()) {
        views[current_view_ix]->unselected();
    }
    auto view = views[view_ix];
    assert(view != nullptr);
    current_view_ix = view_ix;
    view->selected();
    SetWindowTitle(view->buffer()->name.c_str());
}

void Editor::select_buffer(pBuffer const &buffer)
{
    current_view_ix = -1;
    for (int ix = 0; ix < views.size(); ++ix) {
        if (views[ix]->buffer() == buffer) {
            select_view(ix);
            return;
        }
    }
    for (int ix = 0; ix < views.size(); ++ix) {
        if (views[ix]->buffer()->name.empty() && views[ix]->buffer()->text.empty()) {
            auto const &view = Widget::make<BufferView>(self<Editor>(), buffer);
            views[ix] = view;
            select_view(ix);
            return;
        }
    }
    auto const &view = Widget::make<BufferView>(self<Editor>(), buffer);
    views.push_back(view);
    view->parent = self<Widget>();
    ++view->buffer()->version;
#if 0
    if (buffer->mode) {
        view->mode_data = (Widget *) mode_make_data(buffer->mode);
        view->mode_data->parent = (Widget *) view;
    }
    lsp_on_open(buffer);
    lsp_semantic_tokens(buffer);
    //    if (sv_endswith(buffer->name, sv_from(".c")) || sv_endswith(buffer->name, sv_from(".h"))) {
    //        view->mode = (Widget *) widget_new_with_parent(CMode, view);
    //        ++buffer->version;
    //    } else if (sv_endswith(buffer->name, sv_from(".scribble"))) {
    //        view->mode = (Widget *) widget_new_with_parent(ScribbleMode, view);
    //        ++buffer->version;
    //    }
#endif
    select_view(static_cast<int>(views.size()) - 1);
}

void Editor::close_view()
{
    views.erase(views.begin() + current_view_ix);
    if (current_view_ix > 0) {
        select_view(current_view_ix - 1);
    } else if (!views.empty()) {
        select_view(current_view_ix);
    } else {
        new_buffer();
    }
}

void Editor::close_buffer()
{
    auto const &view = current_view();
    auto const &buffer = view->buffer();
    close_view();
    buffer->close();
}

pBufferView Editor::current_view()
{
    assert(current_view_ix >= 0 && current_view_ix < views.size());
    return views[current_view_ix];
}

pBuffer Editor::current_buffer()
{
    assert(current_view_ix >= 0 && current_view_ix < views.size());
    auto &view = views[current_view_ix];
    return view->buffer();
}

void cmd_switch_buffer(pEditor const &editor, JSONValue const &)
{
    struct BufferList : public ListBox<pBuffer> {
        pEditor editor;
        explicit BufferList(pEditor editor)
            : ListBox("Select buffer")
            , editor(std::move(editor))
        {
            for (auto const &buffer : Eddy::the()->buffers) {
                std::string text { buffer->name };
                if (buffer->saved_version < buffer->version) {
                    text += " *";
                }
                entries.emplace_back(text, buffer);
            }
        }

        void submit() override
        {
            editor->select_buffer(entries[selection].payload);
        }
    };
    auto const &listbox = Widget::make<BufferList>(editor);
    listbox->show();
}

void are_you_sure(pEditor const &editor, QueryOption selection)
{
    switch (selection) {
    case QueryOptionYes: {
        auto const &view = editor->current_view();
        auto const &buffer = view->buffer();
        buffer->save();
    } // Fall through:
    case QueryOptionNo:
        editor->close_buffer();
        break;
    default:
        // do nothing
        break;
    }
}

void cmd_close_buffer(pEditor const &editor, JSONValue const &)
{
    auto const &view = editor->current_view();
    auto const &buffer = view->buffer();

    std::string prompt {};
    if (buffer->name.empty() && !buffer->text.empty()) {
        prompt = "File is modified. Do you want to save it before closing?";
    }
    if (buffer->saved_version < buffer->version) {
        prompt = std::format("File '{}' is modified. Do you want to save it before closing?", buffer->name);
    }
    if (!prompt.empty()) {
        query_box<Editor>(editor, prompt, are_you_sure, QueryOptionYesNoCancel);
        return;
    }
    editor->close_buffer();
}

void cmd_close_view(pEditor const &editor, JSONValue const &)
{
    editor->close_view();
}

/*
 * ---------------------------------------------------------------------------
 * Life cycle
 * ---------------------------------------------------------------------------
 */

Editor::Editor(pWidget const& parent)
    : Widget(parent, SizePolicy::Stretch, 0.0)
{
    background = DARKGRAY; // colour_to_color(Eddy::the()->theme.editor.bg);
    padding = Rect<float>(PADDING);
}

void Editor::initialize()
{
    add_command<Editor>("editor-switch-buffer", cmd_switch_buffer)
        .bind(KeyCombo { KEY_B, KModSuper });
    add_command<Editor>("editor-close-buffer", cmd_close_buffer)
        .bind(KeyCombo { KEY_W, KModControl });
    add_command<Editor>("editor-close-view", cmd_close_view)
        .bind(KeyCombo { KEY_W, KModControl | KModShift });
}

void Editor::resize()
{
    columns = (int) ((viewport.width - 2 * PADDING) / Eddy::the()->cell.x);
    lines = (int) ((viewport.height - 2 * PADDING) / Eddy::the()->cell.y);
    for (auto& view : views) {
        view->viewport = viewport;
    }
}

void Editor::draw()
{
    current_view()->draw();
}

void Editor::process_input()
{
    current_view()->process_input();
}

}
