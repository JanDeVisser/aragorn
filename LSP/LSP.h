/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <App/Aragorn.h>
#include <App/Widget.h>
#include <LSP/Schema/CompletionItem.h>
#include <LSP/Schema/ServerCapabilities.h>
#include <LibCore/JSON.h>
#include <LibCore/Lexer.h>
#include <LibCore/Process.h>
#include <LibCore/StringScanner.h>

namespace LSP {

using namespace LibCore;
using namespace Aragorn;

struct Notification {
    std::string              method;
    std::optional<JSONValue> params;

    Notification() = default;

    JSONValue                    encode() const;
    static Decoded<Notification> decode(JSONValue const &value);
};

template<typename MethodParams>
Decoded<MethodParams> make_notification_params(Notification const &notification)
{
    return MethodParams::decode(*notification.params);
}

struct Request {
    static int               next_id;
    pWidget                  sender { nullptr };
    int                      id;
    std::string              method;
    std::optional<JSONValue> params;

    Request()
        : id(next_id++)
    {
    }

    explicit Request(int id)
        : id(id)
    {
    }

    JSONValue encode() const
    {
        JSONValue ret { JSONValue::object() };
        ret.set("jsonrpc", JSONValue { "2.0" });
        ret.set("id", JSONValue { id });
        ret.set("method", JSONValue { method });
        if (params) {
            ret.set("params", *params);
        }
        return ret;
    }
};

template<typename MethodParams>
Request make_request(pWidget const &sender, std::string_view method, std::optional<MethodParams> const &params)
{
    Request req;
    req.sender = sender;
    req.method = method;
    if (params) {
        req.params = params.encode();
    }
    return req;
}

using Requests = std::vector<Request>;

struct Response {
    int                      id;
    std::optional<JSONValue> result;
    std::optional<JSONValue> error_value;

    Response(int id)
        : id(id)
    {
    }

    bool success() const { return result.has_value(); }
    bool error() const { return error_value.has_value(); }

    static Decoded<Response> decode(JSONValue const &value);
};

template<typename MethodParams>
Decoded<MethodParams> make_response_result(Response const &response)
{
    if (!response.success()) {
        return JSONError { JSONError::Code::ProtocolError, "Response returned error" };
    }
    return MethodParams::decode(*response.result);
}

struct LSP;

typedef struct mode *(*LSPInitMode)(LSP *);

struct LSPHandler {
    Process<int> &lsp_start(LSP &lsp);
};

using LSPHandlers = std::vector<LSPHandler>;
using LSPScanner = StringScanner<char>;

struct LSP : Widget {
    LSPHandlers                   handlers;
    std::mutex                    mutex;
    std::condition_variable       init_condition;
    std::optional<Process<LSP *>> lsp;
    ServerCapabilities            server_capabilities;
    Requests                      request_queue;
    std::string                   read_buffer;
    LSPScanner                    scanner;

    LSP()
        : Widget(::Aragorn::Aragorn::the())
    {
    }

    void   initialize() override;
    CError notification(std::string_view method, std::optional<JSONValue> params);
    CError message(pWidget const &sender, std::string_view method, std::optional<JSONValue> params);
    void   initialize_theme();
    void   read(ReadPipe<LSP *> &pipe);
    void   on_initialize_response(JSONValue const &response_json);

private:
    void   initialize_theme_internal();
    CError private_message(pWidget const &sender, std::string_view method, std::optional<JSONValue> params = {});
    CError private_notification(std::string_view method, std::optional<JSONValue> params = {});

    bool m_ready { false };
};

template<typename MethodParams>
CError send_notification(LSP &lsp, std::string_view method, MethodParams const &params)
{
    return lsp.notification(method, LibCore::encode(params));
}

inline CError send_notification(LSP &lsp, std::string_view method)
{
    return lsp.notification(method, {});
}

template<typename MethodParams>
CError send_message(LSP &lsp, pWidget const &sender, std::string_view method, MethodParams const &params)
{
    return lsp.message(sender, method, LibCore::encode(params));
}

inline CError send_message(LSP &lsp, pWidget const &sender, std::string_view method)
{
    return lsp.message(sender, method, {});
}

}

namespace LibCore {

using namespace LSP;

}
