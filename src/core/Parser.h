#pragma once

#include "core/Ast.h"
#include "core/Token.h"

#include <string>
#include <string_view>
#include <vector>

namespace calc::core {

/// Stage 2 of evaluation: a recursive-descent parser that turns tokens into an AST.
///
/// Grammar, from lowest to highest precedence:
///
///     statement  := IDENT '=' expression | expression
///     expression := term (('+' | '-') term)*
///     term       := unary (('*' | '/') unary | implicit)*     implicit: 2x, 3(4+1)
///     unary      := ('-' | '+') unary | power
///     power      := postfix ('^' unary)?                      right-associative
///     postfix    := primary '!'*
///     primary    := NUMBER | IDENT | IDENT '(' arguments ')' | '(' expression ')'
///
/// Each rule is one member function, so the code reads like the grammar.
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    /// @throws CalcError (ErrorKind::Syntax) with the position of the offending token.
    NodePtr parse();

private:
    NodePtr statement();
    NodePtr expression();
    NodePtr term();
    NodePtr unary();
    NodePtr power();
    NodePtr postfix();
    NodePtr primary();
    NodePtr call(const Token& name);

    const Token& peek(std::size_t offset = 0) const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& expect(TokenType type, const std::string& message);
    [[noreturn]] void fail(const std::string& message, const Token& at) const;

    std::vector<Token> tokens_;
    std::size_t current_ = 0;
};

/// Convenience: tokenize and parse in one call.
NodePtr parse(std::string_view input);

}  // namespace calc::core
