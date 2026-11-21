#include "core/Lexer.h"

#include "core/Errors.h"

#include <algorithm>
#include <locale>
#include <sstream>
#include <string>

namespace calc::core {
namespace {

// ASCII-only classification: std::isalpha & co. depend on the C locale, which Qt changes.
bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isLetter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

std::size_t utf8SequenceLength(char lead)
{
    const auto byte = static_cast<unsigned char>(lead);
    if (byte < 0x80) return 1;
    if ((byte >> 5) == 0x6) return 2;
    if ((byte >> 4) == 0xE) return 3;
    if ((byte >> 3) == 0x1E) return 4;
    return 1;
}

/// Locale-independent number parsing ("3.5" must never depend on the user's decimal comma).
double parseNumber(const std::string& text, std::size_t position)
{
    std::istringstream stream(text);
    stream.imbue(std::locale::classic());
    double value = 0.0;
    stream >> value;
    if (stream.fail()) {
        throw CalcError(ErrorKind::Syntax, "'" + text + "' is not a number the calculator can represent", position);
    }
    return value;
}

struct UnicodeAlias {
    std::string_view utf8;
    TokenType type;
    std::string_view normalized;
};

constexpr UnicodeAlias kUnicodeAliases[] = {
    {"×", TokenType::Star, "×"},   // ×
    {"÷", TokenType::Slash, "÷"},  // ÷
    {"−", TokenType::Minus, "−"},  // −
    {"π", TokenType::Identifier, "pi"}, // π
    {"τ", TokenType::Identifier, "tau"},// τ
    {"φ", TokenType::Identifier, "phi"},// φ
};

}  // namespace

std::vector<Token> Lexer::tokenize(std::string_view input)
{
    std::vector<Token> tokens;
    std::size_t i = 0;       // byte index into the UTF-8 input
    std::size_t column = 0;  // character index, used for error positions

    auto push = [&tokens](TokenType type, std::string text, std::size_t position, double value = 0.0) {
        tokens.push_back(Token{type, std::move(text), value, position});
    };

    while (i < input.size()) {
        const char c = input[i];

        if (isSpace(c)) {
            ++i;
            ++column;
            continue;
        }

        // Numbers: 42, 3.14, .5, 6.02e23, 1e-3
        if (isDigit(c) || (c == '.' && i + 1 < input.size() && isDigit(input[i + 1]))) {
            const std::size_t start = i;
            while (i < input.size() && isDigit(input[i])) ++i;
            if (i < input.size() && input[i] == '.') {
                ++i;
                while (i < input.size() && isDigit(input[i])) ++i;
            }
            if (i < input.size() && input[i] == '.') {
                throw CalcError(ErrorKind::Syntax, "A number can contain only one decimal point", column + (i - start));
            }
            // Scientific notation only when a digit follows, so that "2e" still means 2 × e.
            if (i < input.size() && (input[i] == 'e' || input[i] == 'E')) {
                std::size_t j = i + 1;
                if (j < input.size() && (input[j] == '+' || input[j] == '-')) ++j;
                if (j < input.size() && isDigit(input[j])) {
                    i = j;
                    while (i < input.size() && isDigit(input[i])) ++i;
                }
            }
            std::string text(input.substr(start, i - start));
            const double value = parseNumber(text, column);
            push(TokenType::Number, std::move(text), column, value);
            column += i - start;
            continue;
        }

        if (isLetter(c)) {
            const std::size_t start = i;
            while (i < input.size() && (isLetter(input[i]) || isDigit(input[i]))) ++i;
            push(TokenType::Identifier, std::string(input.substr(start, i - start)), column);
            column += i - start;
            continue;
        }

        bool matchedAlias = false;
        for (const UnicodeAlias& alias : kUnicodeAliases) {
            if (input.substr(i, alias.utf8.size()) == alias.utf8) {
                push(alias.type, std::string(alias.normalized), column);
                i += alias.utf8.size();
                ++column;
                matchedAlias = true;
                break;
            }
        }
        if (matchedAlias) continue;

        // "**" is accepted as power, like in Python.
        if (c == '*' && i + 1 < input.size() && input[i + 1] == '*') {
            push(TokenType::Caret, "**", column);
            i += 2;
            column += 2;
            continue;
        }

        TokenType type = TokenType::End;
        switch (c) {
        case '+': type = TokenType::Plus; break;
        case '-': type = TokenType::Minus; break;
        case '*': type = TokenType::Star; break;
        case '/': type = TokenType::Slash; break;
        case '^': type = TokenType::Caret; break;
        case '!': type = TokenType::Bang; break;
        case '(': type = TokenType::LeftParen; break;
        case ')': type = TokenType::RightParen; break;
        case ',': type = TokenType::Comma; break;
        case '=': type = TokenType::Equals; break;
        default: {
            const std::size_t length = std::min(utf8SequenceLength(c), input.size() - i);
            throw CalcError(ErrorKind::Syntax,
                            "The character '" + std::string(input.substr(i, length)) + "' is not understood",
                            column);
        }
        }
        push(type, std::string(1, c), column);
        ++i;
        ++column;
    }

    push(TokenType::End, "", column);
    return tokens;
}

}  // namespace calc::core
