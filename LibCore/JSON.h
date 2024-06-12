/*
 * Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <LibCore/Logging.h>
#include <LibCore/Result.h>

namespace LibCore {

template<typename T>
concept Boolean = std::is_same_v<std::remove_cvref_t<T>, bool>;

template<typename T>
concept Integer = (std::is_integral_v<T> && !Boolean<T>);

template<typename T>
concept UnsignedInteger = (std::is_integral_v<T> && std::is_unsigned_v<T> && !Boolean<T>);

template<typename T>
concept SignedInteger = (std::is_integral_v<T> && std::is_signed_v<T> && !Boolean<T>);

template<typename T>
concept String = (std::is_same_v<std::remove_cvref_t<T>, std::string> || std::is_same_v<std::remove_cvref_t<T>, std::string_view>);

template<Integer Int>
consteval uint64_t max_value()
{
    return 0;
}

template<>
consteval uint64_t max_value<uint64_t>()
{
    return UINT64_MAX;
}

template<>
consteval uint64_t max_value<int64_t>()
{
    return INT64_MAX;
}

template<>
consteval uint64_t max_value<uint32_t>()
{
    return UINT32_MAX;
}

template<>
consteval uint64_t max_value<int32_t>()
{
    return INT32_MAX;
}

template<>
consteval uint64_t max_value<uint16_t>()
{
    return UINT16_MAX;
}

template<>
consteval uint64_t max_value<int16_t>()
{
    return INT16_MAX;
}

template<>
consteval uint64_t max_value<uint8_t>()
{
    return UINT8_MAX;
}

template<>
consteval uint64_t max_value<int8_t>()
{
    return INT8_MAX;
}

template<SignedInteger Int>
consteval int64_t min_value()
{
    abort();
    return 0; // keep clangd happy.
}

template<UnsignedInteger Int>
consteval int64_t min_value()
{
    return 0;
}

template<>
consteval int64_t min_value<int64_t>()
{
    return INT64_MIN;
}

template<>
consteval int64_t min_value<int32_t>()
{
    return INT32_MIN;
}

template<>
consteval int64_t min_value<int16_t>()
{
    return INT16_MIN;
}

template<>
consteval int64_t min_value<int8_t>()
{
    return INT8_MIN;
}

#define JSONTYPES(S) \
    S(Null)          \
    S(String)        \
    S(Integer)       \
    S(Boolean)       \
    S(Double)        \
    S(Array)         \
    S(Object)

enum class JSONType {
#undef S
#define S(code) code,
    JSONTYPES(S)
#undef S
};

static inline char const *JSONType_name(JSONType type)
{
    switch (type) {
#undef S
#define S(type)          \
    case JSONType::type: \
        return #type;
        JSONTYPES(S)
#undef S
    }
}

#define JSONERRORCODES(S) \
    S(NoSuchKey)          \
    S(TypeMismatch)       \
    S(MissingValue)       \
    S(UnexpectedValue)    \
    S(ProtocolError)      \
    S(SyntaxError)

class JSONError {
public:
    enum class Code {
#undef S
#define S(code) code,
        JSONERRORCODES(S)
#undef S
    };

    static char const *JSONErrorCode_name(Code error)
    {
        switch (error) {
#undef S
#define S(code)      \
    case Code::code: \
        return #code;
            JSONERRORCODES(S)
#undef S
        }
        UNREACHABLE();
    }

    Code        code;
    std::string description;

    struct Location {
        int line;
        int column;
    };
    std::optional<Location> location;

    JSONError(Code code, std::string_view const &descr, int line = -1, int column = -1)
        : code(code)
        , description(descr)
        , location()
    {
        if (line >= 0 && column >= 0) {
            location = Location { .line = line, .column = column };
        }
    }

    [[nodiscard]] std::string to_string() const
    {
        std::string prefix {};
        if (location.has_value()) {
            prefix = std::format("Line {}, Column {}: ", location->line, location->column);
        }
        return std::format("{}JSON error {}: {}", prefix, JSONErrorCode_name(code), description);
    }

    static JSONError expected(JSONType expected, JSONType got, std::string_view const &what, int line = -1, int column = -1)
    {
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("'{}': Expected a JSON '{}', got a '{}'", what, JSONType_name(expected), JSONType_name(got)),
            line,
            column,
        };
    }
};

#define CHECK_JSON_TYPE(expr, T)                                 \
    do {                                                         \
        auto t__ = (expr).type();                                \
        if (t__ != JSONType::T) {                                \
            return JSONError::expected(JSONType::T, t__, #expr); \
        }                                                        \
    } while (0)

#define ASSERT_JSON_TYPE(expr, T)                   \
    do {                                            \
        auto t__ = (expr).type();                   \
        assert_with_msg(t__ == JSONType::T, #expr); \
    } while (0)

class JSONValue {
public:
    using Array = std::vector<JSONValue>;
    using Object = std::map<std::string, JSONValue>;

    JSONValue() = default;
    JSONValue(JSONValue const &) = default;

    JSONValue(JSONType type)
        : m_type(type)
    {
        switch (m_type) {
        case JSONType::Null:
        case JSONType::Integer:
            m_value = 0;
            break;
        case JSONType::Boolean:
            m_value = false;
            break;
        case JSONType::String:
            m_value = "";
            break;
        case JSONType::Double:
            m_value = 0.0;
            break;
        case JSONType::Array:
            m_value = Array();
            break;
        case JSONType::Object:
            m_value = Object();
            break;
        }
    }

    JSONValue(std::string_view const &value)
        : m_type(JSONType::String)
        , m_value(std::string(value))
    {
    }

    JSONValue(std::string value)
        : m_type(JSONType::String)
        , m_value(std::move(value))
    {
    }

    JSONValue(char const *value)
        : m_type(JSONType::String)
        , m_value(std::string(value))
    {
    }

    template<Integer Int>
    JSONValue(Int value)
        : m_type(JSONType::Integer)
        , m_value(static_cast<int64_t>(value))
    {
    }

    template<Boolean B>
    JSONValue(B value)
        : m_type(JSONType::Boolean)
        , m_value(value)
    {
    }

    template<std::floating_point Float>
    JSONValue(Float value)
        : m_type(JSONType::Double)
        , m_value(value)
    {
    }

    [[nodiscard]] JSONType type() const { return m_type; }
    [[nodiscard]] bool     is_null() const { return m_type == JSONType::Null; }
    [[nodiscard]] bool     is_string() const { return m_type == JSONType::String; }
    [[nodiscard]] bool     is_integer() const { return m_type == JSONType::Integer; }
    [[nodiscard]] bool     is_boolean() const { return m_type == JSONType::Boolean; }
    [[nodiscard]] bool     is_double() const { return m_type == JSONType::Double; }
    [[nodiscard]] bool     is_array() const { return m_type == JSONType::Array; }
    [[nodiscard]] bool     is_object() const { return m_type == JSONType::Object; }

    static JSONValue array()
    {
        return JSONValue { JSONType::Array };
    }

    static JSONValue object()
    {
        return JSONValue { JSONType::Object };
    }

    friend std::ostream &operator<<(std::ostream &os, JSONValue const &value)
    {
        os << value.serialize();
        return os;
    }

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::string serialize(bool pretty = false, int indent_width = 4, int indent = 0) const;

    template<typename Target>
    using DecodeResult = Result<Target, JSONError>;

    template<typename Target>
    [[nodiscard]] DecodeResult<Target> convert() const
    {
        if (!is_object())
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Cannot convert JSON value of type {} to {}", JSONType_name(type()), typeid(Target).name()),
            };
        return Target::decode_json(*this);
    }

    template<Integer Int>
    [[nodiscard]] DecodeResult<Int> convert() const
    {
        auto v = value<Int>(*this);
        if (!v)
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Cannot convert JSON value '{}' to integer", to_string()),
            };
        return *v;
    }

    template<String Str>
    [[nodiscard]] DecodeResult<Str> convert() const
    {
        return to_string();
    }

    template<Boolean Bool>
    [[nodiscard]] DecodeResult<Bool> convert() const
    {
        auto v = value<Bool>(*this);
        if (!v)
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Cannot convert JSON value '{}' to bool", to_string()),
            };
        return *v;
    }

    template<std::floating_point Float>
    [[nodiscard]] DecodeResult<Float> convert() const
    {
        auto v = value<Float>(*this);
        if (!v)
            return JSONError {
                JSONError::Code::TypeMismatch,
                std::format("Cannot convert JSON value '{}' to floating point", to_string()),
            };
        return *v;
    }

    [[nodiscard]] std::optional<Array> to_array() const
    {
        if (m_type != JSONType::Array)
            return {};
        return std::get<Array>(m_value);
    }

    [[nodiscard]] std::optional<Object> to_object() const
    {
        if (m_type != JSONType::Object)
            return {};
        return std::get<Object>(m_value);
    }

    void append(JSONValue const &value)
    {
        if (m_type != JSONType::Array)
            return;
        auto &array = std::get<Array>(m_value);
        array.push_back(value);
    }

    JSONValue &operator+=(JSONValue const &value)
    {
        append(value);
        return *this;
    }

    void set(std::string_view const &key, JSONValue const &value)
    {
        if (m_type != JSONType::Object)
            return;
        auto &object = std::get<Object>(m_value);
        object[std::string(key)] = value;
    }

    [[nodiscard]] size_t size() const
    {
        switch (m_type) {
        case JSONType::Array:
            return std::get<Array>(m_value).size();
        case JSONType::Object:
            return std::get<Object>(m_value).size();
        default:
            return 0;
        }
    }

    [[nodiscard]] bool empty() const
    {
        switch (m_type) {
        case JSONType::Array:
        case JSONType::Object:
            return size() == 0;
        default:
            return false;
        }
    }

    [[nodiscard]] bool has(std::string_view const &key) const
    {
        if (m_type != JSONType::Object)
            return false;
        auto &object = std::get<Object>(m_value);
        return object.contains(std::string(key));
    }

    [[nodiscard]] std::optional<JSONValue> get(std::string_view const &key) const
    {
        if (m_type != JSONType::Object)
            return {};
        auto &object = std::get<Object>(m_value);
        auto  s = std::string(key);
        if (object.contains(s))
            return object.at(s);
        return {};
    }

    [[nodiscard]] JSONValue get_with_default(std::string_view const &key, JSONType defaultType = JSONType::Object) const
    {
        auto make_default = [defaultType]() {
            return JSONValue(defaultType);
        };

        if (m_type != JSONType::Object) {
            return make_default();
        }
        auto &object = std::get<Object>(m_value);
        auto  s = std::string(key);
        if (object.contains(s)) {
            return object.at(s);
        }
        return make_default();
    }

    template<typename T>
    [[nodiscard]] Result<T, JSONError> try_get(std::string_view const &key) const
    {
        if (!is_object()) {
            return JSONError { JSONError::Code::TypeMismatch, "" };
        }
        auto maybe = get(key);
        if (!maybe.has_value()) {
            return JSONError { JSONError::Code::MissingValue, std::string(key) };
        }
        JSONValue const jv = maybe.value();
        T               v;
        TRY(decode_value<T>(maybe.value(), v));
        return v;
    }

    JSONValue &operator[](std::string_view const &key)
    {
        if (m_type != JSONType::Object) {
            fatal("JSON::operator[str] called on non-object");
        }
        auto             &obj = std::get<Object>(m_value);
        std::string const s { key };
        if (!obj.contains(s)) {
            obj[s] = object();
        }
        return obj[s];
    }

    [[nodiscard]] auto obj_begin() const
    {
        if (is_object()) {
            auto &obj = std::get<Object>(m_value);
            return obj.begin();
        }
        abort();
    }

    [[nodiscard]] auto obj_end() const
    {
        if (is_object()) {
            auto &obj = std::get<Object>(m_value);
            return obj.end();
        }
        abort();
    }

    JSONValue const &operator[](std::string_view const &key) const
    {
        if (m_type != JSONType::Object) {
            fatal("JSON::operator[str] called on non-object");
        }
        auto             &obj = std::get<Object>(m_value);
        std::string const s { key };
        if (!obj.contains(s)) {
            fatal("JSON::operator[str] const called with non-existing key");
        }
        return obj.at(s);
    }

    [[nodiscard]] std::optional<JSONValue> get(unsigned int ix) const
    {
        if (m_type != JSONType::Array)
            return {};
        auto &array = std::get<Array>(m_value);
        if (ix < array.size())
            return array[ix];
        return {};
    }

    [[nodiscard]] JSONValue const &must_get(unsigned int ix) const
    {
        if (m_type != JSONType::Array) {
            fatal("JSON::must_get called on non-array");
        }
        auto &array = std::get<Array>(m_value);
        if (ix < array.size()) {
            return array[ix];
        }
        fatal("Index out of bound ({} <= {}) in JSON::must_get", array.size(), ix);
    }

    [[nodiscard]] JSONValue const &operator[](unsigned int ix) const
    {
        if (m_type != JSONType::Array) {
            fatal("JSON::must_get called on non-array");
        }
        auto &array = std::get<Array>(m_value);
        if (ix < array.size()) {
            return array[ix];
        }
        fatal("Index out of bound ({} <= {}) in JSON::operator[]", array.size(), ix);
    }

    [[nodiscard]] JSONValue &operator[](unsigned int ix)
    {
        if (m_type != JSONType::Array) {
            fatal("JSON::must_get called on non-array");
        }
        auto &array = std::get<Array>(m_value);
        if (ix < array.size()) {
            return array[ix];
        }
        fatal("Index out of bound ({} <= {}) in JSON::operator[]", array.size(), ix);
    }

    [[nodiscard]] auto begin() const
    {
        if (is_array()) {
            auto &obj = std::get<Array>(m_value);
            return obj.begin();
        }
        abort();
    }

    [[nodiscard]] auto end() const
    {
        if (is_array()) {
            auto &obj = std::get<Array>(m_value);
            return obj.end();
        }
        abort();
    }

    [[nodiscard]] auto begin()
    {
        if (is_array()) {
            auto &obj = std::get<Array>(m_value);
            return obj.begin();
        }
        abort();
    }

    [[nodiscard]] auto end()
    {
        if (is_array()) {
            auto &obj = std::get<Array>(m_value);
            return obj.end();
        }
        abort();
    }

    JSONValue &merge(JSONValue const &other)
    {
        switch (type()) {
        case JSONType::Object: {
            if (!other.is_object()) {
                return *this;
            }
            auto const &obj = std::get<Object>(other.m_value);
            for (auto const &[name, value] : obj) {
                if (value.is_object() || value.is_array() && has(name)) {
                    auto my_value = (*this)[name];
                    if (my_value.type() == value.type()) {
                        (*this)[name].merge(value);
                        continue;
                    }
                }
                set(name, value);
            }
        } break;
        case JSONType::Array: {
            if (!other.is_array()) {
                return *this;
            }
            auto const &arr = std::get<Array>(other.m_value);
            for (auto const &value : arr) {
                append(value);
            }
        } break;
        default:
            break;
        }
        return *this;
    }
    using JSONValueValue = std::variant<std::string, int64_t, bool, double, Array, Object>;
    [[nodiscard]] JSONValueValue const &raw_value() const { return m_value; }

    using ReadError = std::variant<LibCError, JSONError>;
    static Result<JSONValue, ReadError> read_file(std::string_view const &);
    static Result<JSONValue, JSONError> deserialize(std::string_view const &);

private:
    JSONType       m_type { JSONType::Null };
    JSONValueValue m_value;
};

template<typename T>
concept JSON = std::is_same_v<std::remove_cvref_t<T>, JSONValue>;

// template<typename T>
//[[nodiscard]] inline std::optional<T> value(JSONValue const &json)
//{
//     UNREACHABLE();
// }

template<JSON J>
[[nodiscard]] inline std::optional<J> value(JSONValue const &json)
{
    return json;
}

template<String Str>
[[nodiscard]] inline std::optional<Str> value(JSONValue const &json)
{
    return json.to_string();
}

template<SignedInteger Int>
[[nodiscard]] inline std::optional<Int> value(JSONValue const &json)
{
    switch (json.type()) {
    case JSONType::Integer: {
        auto v = std::get<int64_t>(json.raw_value());
        if ((v < min_value<Int>()) || (static_cast<uint64_t>(v) > max_value<Int>())) {
            return {};
        }
        return static_cast<Int>(v);
    }
    case JSONType::Double: {
        auto v = std::get<double>(json.raw_value());
        if ((v < min_value<Int>()) || (static_cast<uint64_t>(v) > max_value<Int>())) {
            return {};
        }
        return static_cast<Int>(v);
    }
    case JSONType::Boolean:
        return std::get<bool>(json.raw_value()) ? 1 : 0;
    default:
        return {};
    }
}

template<UnsignedInteger UInt>
[[nodiscard]] inline std::optional<UInt> value(JSONValue const &json)
{
    switch (json.type()) {
    case JSONType::Integer: {
        auto v = std::get<int64_t>(json.raw_value());
        if ((v < 0) || (static_cast<uint64_t>(v) > max_value<UInt>())) {
            return {};
        }
        return static_cast<UInt>(v);
    }
    case JSONType::Double: {
        auto v = std::get<double>(json.raw_value());
        if ((v < 0) || (static_cast<uint64_t>(v) > max_value<UInt>())) {
            return {};
        }
        return static_cast<UInt>(v);
    }
    case JSONType::Boolean:
        return std::get<bool>(json.raw_value()) ? 1 : 0;
    default:
        return {};
    }
}

template<std::floating_point Float>
[[nodiscard]] inline std::optional<Float> value(JSONValue const &json)
{
    switch (json.type()) {
    case JSONType::Integer:
        return static_cast<Float>(std::get<int64_t>(json.raw_value()));
    case JSONType::Double:
        return static_cast<Float>(std::get<double>(json.raw_value()));
    default:
        return {};
    }
}

template<Boolean B = bool>
[[nodiscard]] inline std::optional<B> value(JSONValue const &json)
{
    switch (json.type()) {
    case JSONType::Null:
        return false;
    case JSONType::Integer:
        return std::get<int64_t>(json.raw_value()) != 0;
    case JSONType::Boolean:
        return std::get<bool>(json.raw_value());
    default:
        return {};
    }
}

template<typename T>
[[nodiscard]] inline std::optional<std::vector<T>> values(JSONValue const &json)
{
    if (json.type() != JSONType::Array) {
        return {};
    }
    std::vector<T> ret {};
    for (auto const &v : json) {
        T    t;
        auto res = decode_value<T>(v, t);
        if (res.is_error()) {
            return {};
        }
        ret.push_back(t);
    }
    return ret;
}

template<typename T>
inline JSONValue to_json(T const &value)
{
    return value.encode();
}

template<>
inline JSONValue to_json(JSONValue const &value)
{
    return value;
}

template<String Str>
inline JSONValue to_json(Str const &value)
{
    return JSONValue { value };
}

template<Integer Int>
inline JSONValue to_json(Int const &value)
{
    return JSONValue { value };
}

template<Boolean B>
inline JSONValue to_json(B const &value)
{
    return JSONValue { value };
}

template<std::floating_point Float>
inline JSONValue to_json(Float const &value)
{
    return JSONValue { value };
}

template<typename T>
inline JSONValue to_json(std::shared_ptr<T> const &value)
{
    return to_json(*value);
}

template<typename Element>
inline JSONValue to_json(std::vector<Element> const &value)
{
    JSONValue ret = JSONValue::array();
    for (auto const &elem : value) {
        ret.append(to_json<Element>(elem));
    }
    return ret;
}

template<typename Value>
inline JSONValue to_json(std::map<std::string, Value> const &value)
{
    JSONValue ret = JSONValue::object();
    for (auto const &[key, v] : value) {
        ret.set(key, to_json<Value>(v));
    }
    return ret;
}

template<typename T>
inline JSONValue to_json(std::optional<T> const &value)
{
    if (value)
        return to_json(*value);
    return {};
}

template<int N, typename... Ts>
inline JSONValue to_json(std::variant<Ts...> const &value)
{
    if (N == value.index()) {
        return to_json(std::get<N, Ts...>(value));
    }
    if constexpr (N < (sizeof...(Ts) - 1)) {
        return to_json<N + 1, Ts...>(value);
    }
}

template<typename T>
inline void set(JSONValue &obj, std::string const &key, T const &value)
{
    assert(obj.is_object());
    obj.set(key, to_json(value));
}

template<typename T>
inline void set(JSONValue &obj, std::string const &key, std::optional<T> const &value)
{
    assert(obj.is_object());
    if (value.has_value()) {
        auto const &v = value.value();
        set(obj, key, v);
    }
}

template<typename T>
inline Error<JSONError> decode_value(JSONValue const &json, T &target)
{
    target = TRY_EVAL(T::decode(json));
    return {};
}

template<Integer Int>
inline Error<JSONError> decode_value(JSONValue const &json, Int &target)
{
    if (!json.is_integer()) {
        return JSONError { JSONError::Code::TypeMismatch, "" };
    }
    auto v = value<Int>(json);
    if (!v) {
        return JSONError { JSONError::Code::TypeMismatch, "Integer out of range" };
    }
    target = *v;
    return {};
}

template<Boolean Bool>
inline Error<JSONError> decode_value(JSONValue const &json, Bool &target)
{
    if (!json.is_boolean())
        return JSONError { JSONError::Code::TypeMismatch, "" };
    target = *(value<bool>(json));
    return {};
}

template<std::floating_point Float>
inline Error<JSONError> decode_value(JSONValue const &json, Float &target)
{
    if (!json.is_double())
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("Cannot convert JSON value {} to floating point", json.to_string()),
        };
    target = *(value<Float>(json));
    return {};
}

template<String Str>
inline Error<JSONError> decode_value(JSONValue const &value, Str &target)
{
    if (!value.is_string())
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("Cannot convert JSON value {} to string", value.to_string()),
        };
    target = value.to_string();
    return {};
}

template<typename T>
inline Error<JSONError> decode_value(JSONValue const &value, std::vector<T> &target)
{
    if (!value.is_array())
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("Cannot convert JSON value {} to vector", value.to_string()),
        };
    for (auto ix = 0u; ix < value.size(); ++ix) {
        T decoded;
        TRY(decode_value<T>(value[ix], decoded));
        target.push_back(decoded);
    }
    return {};
}

template<typename T>
inline Error<JSONError> decode_value(JSONValue const &value, std::map<std::string, T> &target)
{
    if (!value.is_object())
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("Cannot convert JSON value {} to map", value.to_string()),
        };
    auto map = *(value.to_object());
    for (auto const &[k, _] : map) {
        T decoded;
        TRY(decode<T>(value, k, decoded));
        target[k] = decoded;
    }
    return {};
}

template<int N, typename... Ts>
inline Error<JSONError> decode_value(JSONValue const &value, std::variant<Ts...> &target)
{
    using V = std::variant<Ts...>;
    using T = std::variant_alternative_t<N, V>;
    T    v;
    auto converted_maybe = decode_value<T>(value, v);
    if (!converted_maybe.is_error()) {
        target.template emplace<N>(v);
        return {};
    }
    if constexpr (N > 0)
        return decode_value<N - 1, Ts...>(value, target);
    return JSONError { JSONError::Code::TypeMismatch, "" };
}

template<typename... Ts>
inline Error<JSONError> decode_value(JSONValue const &value, std::variant<Ts...> &target)
{
    //    std::cerr << "decode_value<std::variant<" << to_string<std::type_info>()(typeid(decltype(target))) << ">()\n";
    return decode_value<sizeof...(Ts) - 1, Ts...>(value, target);
}

template<typename T>
inline Error<JSONError> decode(JSONValue const &obj, std::string const &key, T &target)
{
    //    std::cerr << "decode<" << to_string<std::type_info>()(typeid(decltype(target))) << ">(" << key << ")\n";
    assert(obj.is_object());
    auto value = obj.get(key);
    if (!value)
        return JSONError {
            JSONError::Code::TypeMismatch,
            std::format("JSON object has no key '{}'", key),
        };
    auto err_maybe = decode_value(*value, target);
    if (err_maybe.is_error()) {
        auto err = err_maybe.error();
        err.key = key;
        return err;
    }
    return {};
}

template<typename T>
inline Error<JSONError> decode(JSONValue const &obj, std::string const &key, std::optional<T> &target)
{
    //    std::cerr << "decode<std::optional<" << to_string<std::type_info>()(typeid(decltype(target))) << ">>(" << key << ")\n";
    assert(obj.is_object());
    auto value = obj.get(key);
    if (!value) {
        target.reset();
        return {};
    }
    T    decoded;
    auto maybe = decode(obj, key, decoded);
    if (maybe.is_error()) {
        return maybe;
    }
    target = decoded;
    return {};
}
}
