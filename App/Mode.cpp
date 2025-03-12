/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LibCore/Lexer.h>

#include <App/Aragorn.h>
#include <App/Buffer.h>
#include <App/LexerMode.h>
#include <App/Mode.h>
#include <App/Theme.h>

namespace Aragorn {

std::shared_ptr<::LSP::LSP> CLexer::the_lsp { nullptr };

std::shared_ptr<::LSP::LSP> CLexer::lsp()
{
    if (the_lsp == nullptr) {
        the_lsp = Widget::make<::LSP::LSP>();
    }
    return the_lsp;
}

}
