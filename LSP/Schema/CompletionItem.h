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

struct CompletionItem : public LSPObject {
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
    std::optional<LSP::Any>                                  data;

    static Result<CompletionItem, JSONError> decode(JSONValue const &json)
    {
        CompletionItem ret;
        ret.label = TRY_EVAL(json.try_get<std::string>(label));
        if (json.has("labelDetails") {
            ret.labelDetails = TRY_EVAL(json.try_get<CompletionItemLabelDetails>(labelDetails));
        }
        if (json.has("kind") {
            ret.kind = TRY_EVAL(json.try_get<CompletionItemKind>(kind));
        }
        if (json.has("tags") {
            ret.tags = TRY_EVAL(json.try_get<std::vector<CompletionItemTag>>(tags));
        }
        if (json.has("detail") {
            ret.detail = TRY_EVAL(json.try_get<std::string>(detail));
        }
        if (json.has("documentation") {
            ret.documentation = TRY_EVAL(json.try_get<std::variant<std::string, MarkupContent>>(documentation));
        }
        if (json.has("deprecated") {
            ret.deprecated = TRY_EVAL(json.try_get<bool>(deprecated));
        }
        if (json.has("preselect") {
            ret.preselect = TRY_EVAL(json.try_get<bool>(preselect));
        }
        if (json.has("sortText") {
            ret.sortText = TRY_EVAL(json.try_get<std::string>(sortText));
        }
        if (json.has("filterText") {
            ret.filterText = TRY_EVAL(json.try_get<std::string>(filterText));
        }
        if (json.has("insertText") {
            ret.insertText = TRY_EVAL(json.try_get<std::string>(insertText));
        }
        if (json.has("insertTextFormat") {
            ret.insertTextFormat = TRY_EVAL(json.try_get<InsertTextFormat>(insertTextFormat));
        }
        if (json.has("insertTextMode") {
            ret.insertTextMode = TRY_EVAL(json.try_get<InsertTextMode>(insertTextMode));
        }
        if (json.has("textEdit") {
            ret.textEdit = TRY_EVAL(json.try_get<std::variant<TextEdit, InsertReplaceEdit>>(textEdit));
        }
        if (json.has("textEditText") {
            ret.textEditText = TRY_EVAL(json.try_get<std::string>(textEditText));
        }
        if (json.has("additionalTextEdits") {
            ret.additionalTextEdits = TRY_EVAL(json.try_get<std::vector<TextEdit>>(additionalTextEdits));
        }
        if (json.has("commitCharacters") {
            ret.commitCharacters = TRY_EVAL(json.try_get<std::vector<std::string>>(commitCharacters));
        }
        if (json.has("command") {
            ret.command = TRY_EVAL(json.try_get<LSPCommand>(command));
        }
        if (json.has("data") {
            ret.data = TRY_EVAL(json.try_get<LSP::Any>(data));
        }
         return ret;
    }

    JSONValue encode()
    {
        set(ret, "label", encode<std::string>(label));
        set(ret, "labelDetails", encode<std::optional<CompletionItemLabelDetails>>(labelDetails));
        set(ret, "kind", encode<std::optional<CompletionItemKind>>(kind));
        set(ret, "tags", encode<std::optional<std::vector<CompletionItemTag>>>(tags));
        set(ret, "detail", encode<std::optional<std::string>>(detail));
        set(ret, "documentation", encode<std::optional<std::variant<std::string, MarkupContent>>>(documentation));
        set(ret, "deprecated", encode<std::optional<bool>>(deprecated));
        set(ret, "preselect", encode<std::optional<bool>>(preselect));
        set(ret, "sortText", encode<std::optional<std::string>>(sortText));
        set(ret, "filterText", encode<std::optional<std::string>>(filterText));
        set(ret, "insertText", encode<std::optional<std::string>>(insertText));
        set(ret, "insertTextFormat", encode<std::optional<InsertTextFormat>>(insertTextFormat));
        set(ret, "insertTextMode", encode<std::optional<InsertTextMode>>(insertTextMode));
        set(ret, "textEdit", encode<std::optional<std::variant<TextEdit, InsertReplaceEdit>>>(textEdit));
        set(ret, "textEditText", encode<std::optional<std::string>>(textEditText));
        set(ret, "additionalTextEdits", encode<std::optional<std::vector<TextEdit>>>(additionalTextEdits));
        set(ret, "commitCharacters", encode<std::optional<std::vector<std::string>>>(commitCharacters));
        set(ret, "command", encode<std::optional<LSPCommand>>(command));
        set(ret, "data", encode<std::optional<LSP::Any>>(data));
        JSONValue ret;
    };
};

} /* namespace LSP */
