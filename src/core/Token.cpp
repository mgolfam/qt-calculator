#include "core/Token.h"

namespace calc::core {

std::string_view describe(TokenType type)
{
    switch (type) {
    case TokenType::Number: return "Number";
    case TokenType::Identifier: return "Name (variable, constant or function)";
    case TokenType::Plus: return "Operator: add";
    case TokenType::Minus: return "Operator: subtract / negate";
    case TokenType::Star: return "Operator: multiply";
    case TokenType::Slash: return "Operator: divide";
    case TokenType::Caret: return "Operator: power";
    case TokenType::Bang: return "Operator: factorial";
    case TokenType::LeftParen: return "Opening parenthesis";
    case TokenType::RightParen: return "Closing parenthesis";
    case TokenType::Comma: return "Argument separator";
    case TokenType::Equals: return "Assignment";
    case TokenType::End: return "End of input";
    }
    return "Unknown";
}

}  // namespace calc::core
