#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace calc::core {

enum class ErrorKind {
    Syntax,  ///< The text is not a well-formed expression.
    Math,    ///< Well-formed, but mathematically undefined (e.g. 1 ÷ 0).
    Name,    ///< Refers to an unknown variable or function.
};

inline std::string_view describe(ErrorKind kind)
{
    switch (kind) {
    case ErrorKind::Syntax: return "Syntax error";
    case ErrorKind::Math: return "Math error";
    case ErrorKind::Name: return "Unknown name";
    }
    return "Error";
}

/// The single exception type thrown by the core. Messages are written for learners.
class CalcError : public std::runtime_error {
public:
    CalcError(ErrorKind kind, const std::string& message, std::optional<std::size_t> position = std::nullopt)
        : std::runtime_error(message)
        , kind_(kind)
        , position_(position)
    {
    }

    ErrorKind kind() const noexcept { return kind_; }
    std::optional<std::size_t> position() const noexcept { return position_; }

private:
    ErrorKind kind_;
    std::optional<std::size_t> position_;
};

}  // namespace calc::core
