#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace calc::core {

/// Every kind of "word" the lexer can recognise in an expression.
enum class TokenType {
    Number,
    Identifier,
    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    Bang,
    LeftParen,
    RightParen,
    Comma,
    Equals,
    End,
};

/// A short, human readable description of a token type (shown in the Tokens tab).
std::string_view describe(TokenType type);

/// The smallest meaningful piece of an expression, e.g. `3.14`, `sin`, `(` or `×`.
struct Token {
    TokenType type = TokenType::End;
    std::string text;          ///< Normalised source text (e.g. "π" becomes "pi").
    double value = 0.0;        ///< Numeric value; only meaningful for TokenType::Number.
    std::size_t position = 0;  ///< Character offset in the original input.
};

}  // namespace calc::core
