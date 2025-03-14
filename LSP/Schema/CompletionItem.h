/**
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * THIS IS GENERATED CODE. DO NOT MODIFY.
 */

#pragma once

#include <LSP/Schema/CompletionItemKind.h>
#include <LSP/Schema/CompletionItemLabelDetails.h>
#include <LSP/Schema/CompletionItemTag.h>
#include <LSP/Schema/InsertReplaceEdit.h>
#include <LSP/Schema/InsertTextFormat.h>
#include <LSP/Schema/InsertTextMode.h>
#include <LSP/Schema/LSPBase.h>
#include <LSP/Schema/LSPCommand.h>
#include <LSP/Schema/MarkupContent.h>
#include <LSP/Schema/TextEdit.h>

namespace LSP {

struct CompletionItem {
    std::string                                              label;
    std::optional<CompletionItemLabelDetails>                labelDetails;
    std::optional<CompletionItemKind>                        kind;
    std::optional<std::vector<CompletionItemTag>>            tags;
    std::optional<std::string>                               detail;
    std::optional<std::variant<std::string, MarkupContent>>  documentation;
    std::optional<bool>                                      deprecated;
    std::optional<bool>                                      preselect;
    std::optional<std::string>                               sortText;
    std::optional<std::string>                               filterText;
    std::optional<std::string>                               insertText;
    std::optional<InsertTextFormat>                          insertTextFormat;
    std::optional<InsertTextMode>                            insertTextMode;
    std::optional<std::variant<TextEdit, InsertReplaceEdit>> textEdit;
    std::optional<std::string>                               textEditText;
    std::optional<std::vector<TextEdit>>                     additionalTextEdits;
    std::optional<std::vector<std::string>>                  commitCharacters;
    std::optional<LSPCommand>                                command;
    std::optional<Any>                                       data;

    static Decoded<CompletionItem> decode(JSONValue const &json)
    {
        CompletionItem ret;
        ret.label = TRY_EVAL(json.try_get<std::string>("label"));
        if (json.has("labelDetails")) {
            ret.labelDetails = TRY_EVAL(json.try_get<CompletionItemLabelDetails>("labelDetails"));
        }
        if (json.has("kind")) {
            ret.kind = TRY_EVAL(json.try_get<CompletionItemKind>("kind"));
        }
        if (json.has("tags")) {
            ret.tags = TRY_EVAL(json.try_get_array<CompletionItemTag>("tags"));
        }
        if (json.has("detail")) {
            ret.detail = TRY_EVAL(json.try_get<std::string>("detail"));
        }
        if (json.has("documentation")) {
            ret.documentation = TRY_EVAL(json.try_get_variant<std::string, MarkupContent>("documentation"));
        }
        if (json.has("deprecated")) {
            ret.deprecated = TRY_EVAL(json.try_get<bool>("deprecated"));
        }
        if (json.has("preselect")) {
            ret.preselect = TRY_EVAL(json.try_get<bool>("preselect"));
        }
        if (json.has("sortText")) {
            ret.sortText = TRY_EVAL(json.try_get<std::string>("sortText"));
        }
        if (json.has("filterText")) {
            ret.filterText = TRY_EVAL(json.try_get<std::string>("filterText"));
        }
        if (json.has("insertText")) {
            ret.insertText = TRY_EVAL(json.try_get<std::string>("insertText"));
        }
        if (json.has("insertTextFormat")) {
            ret.insertTextFormat = TRY_EVAL(json.try_get<InsertTextFormat>("insertTextFormat"));
        }
        if (json.has("insertTextMode")) {
            ret.insertTextMode = TRY_EVAL(json.try_get<InsertTextMode>("insertTextMode"));
        }
        if (json.has("textEdit")) {
            ret.textEdit = TRY_EVAL(json.try_get_variant<TextEdit, InsertReplaceEdit>("textEdit"));
        }
        if (json.has("textEditText")) {
            ret.textEditText = TRY_EVAL(json.try_get<std::string>("textEditText"));
        }
        if (json.has("additionalTextEdits")) {
            ret.additionalTextEdits = TRY_EVAL(json.try_get_array<TextEdit>("additionalTextEdits"));
        }
        if (json.has("commitCharacters")) {
            ret.commitCharacters = TRY_EVAL(json.try_get_array<std::string>("commitCharacters"));
        }
        if (json.has("command")) {
            ret.command = TRY_EVAL(json.try_get<LSPCommand>("command"));
        }
        if (json.has("data")) {
            ret.data = TRY_EVAL(json.try_get<Any>("data"));
        }
        return std::move(ret);
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONType::Object };
        set(ret, "label", label);
        set(ret, "labelDetails", labelDetails);
        set(ret, "kind", kind);
        set(ret, "tags", tags);
        set(ret, "detail", detail);
        set(ret, "documentation", documentation);
        set(ret, "deprecated", deprecated);
        set(ret, "preselect", preselect);
        set(ret, "sortText", sortText);
        set(ret, "filterText", filterText);
        set(ret, "insertText", insertText);
        set(ret, "insertTextFormat", insertTextFormat);
        set(ret, "insertTextMode", insertTextMode);
        set(ret, "textEdit", textEdit);
        set(ret, "textEditText", textEditText);
        set(ret, "additionalTextEdits", additionalTextEdits);
        set(ret, "commitCharacters", commitCharacters);
        set(ret, "command", command);
        set(ret, "data", data);
        return ret;
    };
};

} /* namespace LSP */
