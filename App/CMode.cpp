/*
 * Copyright (c) 2025, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <App/CMode.h>
#include <LSP/Schema/DidChangeTextDocumentParams.h>
#include <LSP/Schema/DidCloseTextDocumentParams.h>
#include <LSP/Schema/DidOpenTextDocumentParams.h>
#include <LSP/Schema/DidSaveTextDocumentParams.h>
#include <LSP/Schema/SemanticTokensParams.h>
#include <LSP/Schema/TextDocumentContentChangeEvent.h>
#include <LibCore/Utf8.h>

namespace Aragorn {

std::shared_ptr<::LSP::LSP> CLexer::the_lsp { nullptr };

std::shared_ptr<::LSP::LSP> CLexer::lsp()
{
    if (the_lsp == nullptr) {
        the_lsp = Widget::make<::LSP::LSP>();
    }
    return the_lsp;
}

void CLexer::did_open(pBuffer const &buffer)
{
    if (buffer->name.empty()) {
        return;
    }
    DidOpenTextDocumentParams did_open;
    did_open.textDocument.uri = buffer->uri();
    did_open.textDocument.languageId = "c";
    did_open.textDocument.version = 0;
    did_open.textDocument.text = MUST_EVAL(to_utf8(buffer->substr(0)));
    MUST(lsp()->notification("textDocument/didOpen", did_open.encode()));
}

void CLexer::did_change(pBuffer const &buffer, BufferEvent const &ev)
{
    if (buffer->name.empty()) {
        return;
    }
    DidChangeTextDocumentParams    did_change;
    TextDocumentContentChangeEvent contentChange;

    did_change.textDocument.uri = buffer->uri();
    did_change.textDocument.version = buffer->version;
    auto &range = std::get<TextDocumentContentChangeRange>(contentChange);
    range.range.start.line = ev.range.start.line;
    range.range.start.character = ev.range.start.column;
    range.range.end.line = ev.range.end.line;
    range.range.end.character = ev.range.end.column;
    range.text = {};
    switch (ev.type) {
    case BufferEventType::Insert:
        range.text = MUST_EVAL(to_utf8(ev.insert()));
        break;
    case BufferEventType::Replace:
        range.text = MUST_EVAL(to_utf8(ev.replacement().replacement));
        break;
    default:
        break;
    }
    did_change.contentChanges.emplace_back(std::move(contentChange));
    MUST(lsp()->notification("textDocument/didChange", did_change.encode()));
}

void CLexer::semantic_tokens(pBuffer const &buffer)
{
    if (buffer->name.empty()) {
        return;
    }
    SemanticTokensParams semantic_tokens_params;
    semantic_tokens_params.textDocument.uri = buffer->uri();
    MUST(lsp()->message(buffer, "textDocument/semanticTokens/full", semantic_tokens_params.encode()));
}

void CLexer::did_save(pBuffer const &buffer)
{
    if (buffer->name.empty()) {
        return;
    }
    DidSaveTextDocumentParams did_save;
    did_save.textDocument.uri = buffer->uri();
    did_save.text = MUST_EVAL(to_utf8(buffer->substr(0)));
    MUST(lsp()->notification("textDocument/didSave", did_save.encode()));
}

void CLexer::did_close(pBuffer const &buffer)
{
    if (buffer->name.empty()) {
        return;
    }
    DidCloseTextDocumentParams did_close;
    did_close.textDocument.uri = buffer->uri();
    MUST(lsp()->notification("textDocument/didClose", did_close.encode()));
}

}
