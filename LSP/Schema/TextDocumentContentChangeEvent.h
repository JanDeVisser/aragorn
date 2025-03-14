/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/TextDocumentContentChangeRange.h>
#include <LSP/Schema/TextDocumentContentChangeText.h>

namespace LSP {

using TextDocumentContentChangeEvent = std::variant<TextDocumentContentChangeRange, TextDocumentContentChangeText>;

} /* namespace LSP */
