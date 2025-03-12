/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <LSP/LSP.h>
#include <LSP/Schema/InitializeParams.h>
#include <LSP/Schema/InitializeResult.h>

#include <App/Aragorn.h>
#include <LibCore/JSON.h>

namespace LSP {

using namespace Aragorn;
using namespace LibCore;

int Request::next_id { 0 };

JSONValue Notification::encode() const
{
    JSONValue ret;
    ret.set("jsonrpc", JSONValue { "2.0" });
    ret.set("method", JSONValue { method });
    if (params) {
        ret.set("params", params.value());
    }
    return ret;
}

Decoded<Notification> Notification::decode(JSONValue const &json)
{
    Notification ret {};

    ret.method = TRY_EVAL(json.try_get<std::string>("method"));
    ret.params = json.get("params");
    return ret;
}

Decoded<Response> Response::decode(JSONValue const &json)
{
    int      id = TRY_EVAL(json.try_get<int>("id"));
    Response ret { id };
    ret.result = json.get("result");
    ret.error_value = json.get("error");
    assert(ret.success() ^ ret.error());
    return ret;
}

CError LSP::message(pWidget const &sender, std::string_view method, std::optional<JSONValue> params)
{
    Request req;
    req.sender = sender;
    req.method = method;
    req.params = params;
    request_queue.push_back(req);
    auto json = req.encode().serialize();
    auto content_length = std::format("Content-Length: %zu\r\n\r\n", json.length() + 2);
    TRY(lsp->write_to(content_length));
    TRY(lsp->write_to(json));
    TRY(lsp->write_to("\r\n"));
    return {};
}

void handle_initialize_response(pWidget const &lsp, JSONValue const &response_json)
{
    std::dynamic_pointer_cast<LSP>(lsp)->on_initialize_response(response_json);
}

void LSP::on_initialize_response(JSONValue const &response_json)
{
    auto lk = std::unique_lock(mutex);
    init_condition.wait(lk);
    auto response = Response::decode(response_json);
    assert(!response.is_error());
    auto result = make_response_result<InitializeResult>(response.value());
    if (result.value().serverInfo) {
        info(LSP, "LSP server name: {}", result.value().serverInfo->name);
        if (result.value().serverInfo->version) {
            info(LSP, "LSP server version: {}", *(result.value().serverInfo->version));
        }
    }
    server_capabilities = result.value().capabilities;
    initialize_theme_internal();
    MUST(send_notification(*this, "initialized"));
    m_ready = true;
    init_condition.notify_all();
}

void lsp_read(ReadPipe<LSP *> &pipe)
{
    LSP *lsp = pipe.context();
    lsp->read(pipe);
}

void LSP::read(ReadPipe<LSP *> &pipe)
{
    do {
        read_buffer += pipe.current();
        scanner.string = read_buffer;
        do {
            scanner.reset();
            if (!scanner.expect("Content-Length:")) {
                return;
            }
            scanner.skip_whitespace();
            size_t resp_content_length = scanner.read_number();
            if (!resp_content_length) {
                scanner.rewind();
                return;
            }
            if (!scanner.expect("\r\n\r\n")) {
                scanner.rewind();
                return;
            }
            auto response_json = scanner.read(resp_content_length);
            if (response_json.length() < resp_content_length) {
                scanner.rewind();
                return;
            }
            auto ret_maybe = JSONValue::deserialize(response_json);
            if (ret_maybe.is_error()) {
                info(LSP, "ERROR Parsing incoming JSON: {}", ret_maybe.error().description);
                ::Aragorn::Aragorn::the()->set_message(std::format("LSP: {}", ret_maybe.error().description));
                continue;
            }
            JSONValue const &ret = ret_maybe.value();
            if (ret.has("id")) {
                auto response_maybe = Response::decode(ret);
                if (response_maybe.is_error()) {
                    ::Aragorn::Aragorn::the()->set_message(std::format("LSP: {}", response_maybe.error().description));
                    continue;
                }
                auto const &response = response_maybe.value();
                for (Request const &req : request_queue) {
                    if (req.id == response.id) {
                        if (req.method == "initialize") {
                            handle_initialize_response(req.sender, ret);
                            continue;
                        }
                        req.sender->submit(std::format("lsp-{}", req.method), ret);
                    }
                }
                continue;
            }
            assert(ret.has("method"));
            auto notification_maybe = Notification::decode(ret);
            if (notification_maybe.is_error()) {
                ::Aragorn::Aragorn::the()->set_message(std::format("LSP: {}", notification_maybe.error().description));
                continue;
            }
            submit(std::format("lsp-{}", notification_maybe.value().method), ret);
        } while (scanner.point.index < scanner.string.length());
        scanner.point = {};
        read_buffer.clear();
        scanner.string = read_buffer;
        scanner.reset();
    } while (pipe.current().length() > 0);
}

CError LSP::notification(std::string_view method, std::optional<JSONValue> params)
{
    Notification notification;
    notification.method = method;
    notification.params = params;
    auto json = notification.encode().serialize();
    auto content_length = std::format("Content-Length: %zu\r\n\r\n", json.length() + 2);
    TRY(lsp->write_to(content_length));
    TRY(lsp->write_to(json));
    TRY(lsp->write_to("\r\n"));
    return {};
}

void LSP::initialize_theme_internal()
{
    for (auto &tokenType : server_capabilities.semanticTokensProvider->legend.tokenTypes) {
        auto semantic_token_type_maybe = SemanticTokenTypes_from_string(tokenType);
        if (!semantic_token_type_maybe) {
            continue;
        };
        // Theme::the().map_semantic_type(&eddy.theme, i, type);
    }
}

void LSP::initialize_theme()
{
    if (!m_ready) {
        return;
    }
    initialize_theme_internal();
}

void LSP::initialize()
{
    if (m_ready) {
        return;
    }

    std::unique_lock lk(mutex);
    init_condition.wait(lk);
    if (m_ready) {
        lk.unlock();
        init_condition.notify_one();
        return;
    }
    if (lsp) {
        std::unique_lock lk2(mutex);
        init_condition.wait(lk2);
        assert(m_ready);
        return;
    }

    lsp.emplace("clangd", "--use-dirty-headers", "--background-index");
    lsp->stderr_file = "/tmp/clangd.log";
    lsp->on_stdout_read(lsp_read);
    MUST(lsp->background(this));

    InitializeParams params;
    params.processId.emplace<int>(getpid());
    params.clientInfo = InitializeParams::ClientInfo { ARAGORN_NAME, ARAGORN_VERSION };
    params.rootUri.emplace<DocumentUri>(std::format("file://%s", ::Aragorn::Aragorn::the()->project->project_dir));

    TextDocumentSyncClientCapabilities syncCapabilities;
    syncCapabilities.didSave = true;
    syncCapabilities.willSave = false;
    syncCapabilities.willSaveWaitUntil = false;

    SemanticTokensClientCapabilities semanticTokensClientCapabilities;
    semanticTokensClientCapabilities.multilineTokenSupport = true;
    semanticTokensClientCapabilities.requests = SemanticTokensClientCapabilities::Requests {};
    semanticTokensClientCapabilities.requests.full.emplace<bool>(true);
    semanticTokensClientCapabilities.tokenTypes.emplace_back(SemanticTokenTypes_as_string(SemanticTokenTypes::Comment));
    semanticTokensClientCapabilities.tokenTypes.emplace_back(SemanticTokenTypes_as_string(SemanticTokenTypes::Keyword));
    semanticTokensClientCapabilities.tokenTypes.emplace_back(SemanticTokenTypes_as_string(SemanticTokenTypes::Variable));
    semanticTokensClientCapabilities.tokenTypes.emplace_back(SemanticTokenTypes_as_string(SemanticTokenTypes::Type));

    params.capabilities.textDocument = TextDocumentClientCapabilities {};
    params.capabilities.textDocument->synchronization = syncCapabilities;
    params.capabilities.textDocument->semanticTokens = semanticTokensClientCapabilities;

    MUST(send_message(*this, ::Aragorn::Aragorn::the(), "initialize", params));
    init_condition.wait(lk);
}

}
