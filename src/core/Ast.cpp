#include "core/Ast.h"

namespace calc::core {

std::string_view symbol(UnaryOp op)
{
    switch (op) {
    case UnaryOp::Negate: return "−";
    case UnaryOp::Factorial: return "!";
    }
    return "?";
}

std::string_view symbol(BinaryOp op)
{
    switch (op) {
    case BinaryOp::Add: return "+";
    case BinaryOp::Subtract: return "−";
    case BinaryOp::Multiply: return "×";
    case BinaryOp::Divide: return "÷";
    case BinaryOp::Power: return "^";
    }
    return "?";
}

}  // namespace calc::core
