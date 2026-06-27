#pragma once
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

namespace logharbor::core {

enum class ErrorCode {
    ok = 0,
    invalid_argument,
    invalid_format,
    truncated_input,
    overflow,
    not_found,
    io_error,
    unsupported,
    evaluation_error,
    storage_corrupt
};

struct Diagnostic {
    ErrorCode code{ErrorCode::ok};
    std::string message;
    std::size_t offset{0};
    std::string context;

    Diagnostic() = default;
    Diagnostic(ErrorCode c, std::string m, std::size_t o = 0, std::string ctx = {})
        : code(c), message(std::move(m)), offset(o), context(std::move(ctx)) {}

    bool ok() const { return code == ErrorCode::ok; }
    std::string describe() const {
        std::ostringstream out;
        out << message;
        if (offset != 0) out << " at byte " << offset;
        if (!context.empty()) out << " near '" << context << "'";
        return out.str();
    }
};

template <typename T>
class Result {
    bool has_{false};
    T value_{};
    Diagnostic diag_{};
public:
    static Result success(T value) {
        Result r;
        r.has_ = true;
        r.value_ = std::move(value);
        return r;
    }
    static Result failure(Diagnostic d) {
        Result r;
        r.has_ = false;
        r.diag_ = std::move(d);
        return r;
    }
    bool ok() const { return has_; }
    explicit operator bool() const { return has_; }
    T& value() { return value_; }
    const T& value() const { return value_; }
    T&& take() { return std::move(value_); }
    const Diagnostic& error() const { return diag_; }
};

template <>
class Result<void> {
    bool has_{true};
    Diagnostic diag_{};
public:
    static Result success() { return Result{}; }
    static Result failure(Diagnostic d) {
        Result r;
        r.has_ = false;
        r.diag_ = std::move(d);
        return r;
    }
    bool ok() const { return has_; }
    explicit operator bool() const { return has_; }
    const Diagnostic& error() const { return diag_; }
};

inline Diagnostic make_error(ErrorCode code, const std::string& message, std::size_t offset = 0, const std::string& context = {}) {
    return Diagnostic(code, message, offset, context);
}

} // namespace logharbor::core
