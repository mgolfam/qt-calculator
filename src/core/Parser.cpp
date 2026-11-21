#include "core/Parser.h"

#include "core/Errors.h"
#include "core/Lexer.h"

namespace calc::core {

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens))
{
    if (tokens_.empty() || tokens_.back().type != TokenType::End) {
        const std::size_t end = tokens_.empty() ? 0 : tokens_.back().position + tokens_.back().text.size();
        tokens_.push_back(Token{TokenType::End, "", 0.0, end});
    }
}

NodePtr Parser::parse()
{
    current_ = 0;
    if (check(TokenType::End)) fail("The expression is empty", peek());

    NodePtr root = statement();
    if (!check(TokenType::End)) {
        const Token& extra = peek();
        if (extra.type == TokenType::RightParen) fail("There is a ')' without a matching '('", extra);
        fail("Unexpected '" + extra.text + "' — is an operator missing before it?", extra);
    }
    return root;
}

NodePtr Parser::statement()
{
    if (check(TokenType::Identifier) && peek(1).type == TokenType::Equals) {
        const Token name = advance();
        advance();  // '='
        if (check(TokenType::End)) fail("Write a value after '=' — for example " + name.text + " = 5", peek());
        NodePtr value = expression();
        return std::make_unique<AssignNode>(name.text, std::move(value), name.position);
    }
    return expression();
}

NodePtr Parser::expression()
{
    NodePtr left = term();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        const Token op = advance();
        NodePtr right = term();
        const BinaryOp kind = op.type == TokenType::Plus ? BinaryOp::Add : BinaryOp::Subtract;
        left = std::make_unique<BinaryNode>(kind, std::move(left), std::move(right), op.position);
    }
    return left;
}

NodePtr Parser::term()
{
    NodePtr left = unary();
    while (true) {
        if (check(TokenType::Star) || check(TokenType::Slash)) {
            const Token op = advance();
            NodePtr right = unary();
            const BinaryOp kind = op.type == TokenType::Star ? BinaryOp::Multiply : BinaryOp::Divide;
            left = std::make_unique<BinaryNode>(kind, std::move(left), std::move(right), op.position);
        } else if (check(TokenType::Identifier) || check(TokenType::LeftParen)) {
            // Implicit multiplication: "2x", "3(4 + 1)", "2 sin(x)".
            const std::size_t position = peek().position;
            NodePtr right = power();
            left = std::make_unique<BinaryNode>(BinaryOp::Multiply, std::move(left), std::move(right), position, true);
        } else {
            return left;
        }
    }
}

NodePtr Parser::unary()
{
    if (check(TokenType::Minus)) {
        const Token op = advance();
        NodePtr operand = unary();
        return std::make_unique<UnaryNode>(UnaryOp::Negate, std::move(operand), op.position);
    }
    if (match(TokenType::Plus)) return unary();
    return power();
}

NodePtr Parser::power()
{
    NodePtr base = postfix();
    if (check(TokenType::Caret)) {
        const Token op = advance();
        // Recursing through unary() makes ^ right-associative and allows 2^-1.
        NodePtr exponent = unary();
        return std::make_unique<BinaryNode>(BinaryOp::Power, std::move(base), std::move(exponent), op.position);
    }
    return base;
}

NodePtr Parser::postfix()
{
    NodePtr node = primary();
    while (check(TokenType::Bang)) {
        const Token op = advance();
        node = std::make_unique<UnaryNode>(UnaryOp::Factorial, std::move(node), op.position);
    }
    return node;
}

NodePtr Parser::primary()
{
    const Token& token = peek();
    switch (token.type) {
    case TokenType::Number: {
        const Token number = advance();
        return std::make_unique<NumberNode>(number.value, number.text, number.position);
    }
    case TokenType::Identifier: {
        const Token name = advance();
        if (check(TokenType::LeftParen)) return call(name);
        return std::make_unique<VariableNode>(name.text, name.position);
    }
    case TokenType::LeftParen: {
        advance();
        if (check(TokenType::RightParen)) fail("Empty parentheses — put an expression inside ( )", peek());
        NodePtr inner = expression();
        expect(TokenType::RightParen, "Missing ')' — every '(' needs a matching ')'");
        return inner;
    }
    case TokenType::End:
        fail("The expression ends too early — a number, name or '(' should come next", token);
    case TokenType::RightParen:
        fail("Expected a value before ')'", token);
    default:
        fail("Expected a number, a name or '(' but found '" + token.text + "'", token);
    }
}

NodePtr Parser::call(const Token& name)
{
    advance();  // '('
    std::vector<NodePtr> arguments;
    if (!check(TokenType::RightParen)) {
        do {
            arguments.push_back(expression());
        } while (match(TokenType::Comma));
    }
    expect(TokenType::RightParen, "Missing ')' to close " + name.text + "( … )");
    return std::make_unique<CallNode>(name.text, std::move(arguments), name.position);
}

const Token& Parser::peek(std::size_t offset) const
{
    const std::size_t index = current_ + offset;
    return index < tokens_.size() ? tokens_[index] : tokens_.back();
}

const Token& Parser::advance()
{
    const Token& token = peek();
    if (current_ < tokens_.size() - 1) ++current_;
    return token;
}

bool Parser::check(TokenType type) const { return peek().type == type; }

bool Parser::match(TokenType type)
{
    if (!check(type)) return false;
    advance();
    return true;
}

const Token& Parser::expect(TokenType type, const std::string& message)
{
    if (!check(type)) fail(message, peek());
    return advance();
}

void Parser::fail(const std::string& message, const Token& at) const
{
    throw CalcError(ErrorKind::Syntax, message, at.position);
}

NodePtr parse(std::string_view input)
{
    Parser parser(Lexer::tokenize(input));
    return parser.parse();
}

}  // namespace calc::core
