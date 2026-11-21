#include "core/Evaluator.h"

#include "core/Errors.h"
#include "core/NumberFormat.h"

#include <cmath>

namespace calc::core {
namespace {

constexpr double kPi = 3.14159265358979323846;

bool isWhole(double v) { return std::isfinite(v) && std::floor(v) == v; }

/// Negative operands get brackets so "−2 ^ 2" is never ambiguous in a step.
std::string operandText(double v)
{
    const std::string text = formatNumber(v);
    return v < 0 ? "(" + text + ")" : text;
}

double ensureFinite(double value, const Node& node)
{
    if (std::isnan(value)) throw CalcError(ErrorKind::Math, "The result is undefined (not a real number)", node.position);
    if (std::isinf(value)) throw CalcError(ErrorKind::Math, "The result is too large to represent", node.position);
    return value;
}

std::string explainPower(double base, double exponent)
{
    if (exponent == 0.0 && base == 0.0) return "0⁰ is defined as 1 by convention (an empty product)";
    if (exponent == 0.0) return "Any non-zero number to the power 0 is 1";
    if (exponent == 1.0) return "A number to the power 1 is itself";
    if (exponent == 0.5) return "The power ½ is the same as a square root";
    if (isWhole(exponent) && exponent >= 2 && exponent <= 6) {
        std::string text = "Repeated multiplication: ";
        for (int i = 0; i < static_cast<int>(exponent); ++i) {
            if (i > 0) text += " × ";
            text += operandText(base);
        }
        return text;
    }
    if (isWhole(exponent) && exponent < 0) {
        return "A negative power means one divided by the positive power: 1 ÷ " + operandText(base) + "^" +
               formatNumber(-exponent);
    }
    return "Exponentiation";
}

std::string explainFactorial(double n)
{
    if (n == 0.0) return "By definition 0! = 1 (there is exactly one way to arrange nothing)";
    if (n <= 10) {
        std::string text = "Multiply every whole number from n down to 1: ";
        for (int i = static_cast<int>(n); i >= 1; --i) {
            text += std::to_string(i);
            if (i > 1) text += " × ";
        }
        return text;
    }
    return "Multiply every whole number from 1 to " + formatNumber(n);
}

std::string explainDivision(double a, double b)
{
    if (isWhole(a) && isWhole(b) && a >= 0 && b > 0 && a < 1e12 && b < 1e12) {
        const auto dividend = static_cast<long long>(a);
        const auto divisor = static_cast<long long>(b);
        if (dividend % divisor != 0) {
            return "Division: " + std::to_string(divisor) + " fits into " + std::to_string(dividend) + " " +
                   std::to_string(dividend / divisor) + " whole times, remainder " + std::to_string(dividend % divisor);
        }
    }
    return "Division";
}

std::string explainBinary(const BinaryNode& node, double left, double right)
{
    switch (node.op) {
    case BinaryOp::Add: return "Addition";
    case BinaryOp::Subtract: return "Subtraction";
    case BinaryOp::Multiply:
        return node.implicit ? "Implicit multiplication — writing 2x means 2 × x" : "Multiplication";
    case BinaryOp::Divide: return explainDivision(left, right);
    case BinaryOp::Power: return explainPower(left, right);
    }
    return {};
}

}  // namespace

Evaluator::Evaluator(Environment& environment, const MathLibrary& library, bool recordSteps)
    : environment_(environment)
    , library_(library)
    , recordSteps_(recordSteps)
{
}

double Evaluator::evaluate(const Node& root)
{
    steps_.clear();
    return evaluateChild(root);
}

double Evaluator::evaluateChild(const Node& node)
{
    node.accept(*this);
    return result_;
}

void Evaluator::record(StepKind kind, std::string expression, double value, std::string explanation)
{
    if (!recordSteps_) return;
    steps_.push_back(EvaluationStep{kind, std::move(expression), formatNumber(value), std::move(explanation)});
}

void Evaluator::visit(const NumberNode& node) { result_ = node.value; }

void Evaluator::visit(const VariableNode& node)
{
    if (const auto value = environment_.variable(node.name)) {
        record(StepKind::Lookup, node.name, *value, "Value stored in the variable '" + node.name + "'");
        result_ = *value;
        return;
    }
    if (const ConstantInfo* constant = library_.findConstant(node.name)) {
        record(StepKind::Lookup, constant->symbol, constant->value, constant->description);
        result_ = constant->value;
        return;
    }
    if (library_.findFunction(node.name) != nullptr) {
        throw CalcError(ErrorKind::Name, "'" + node.name + "' is a function — call it with brackets, like " +
                                             node.name + "(…)", node.position);
    }
    throw CalcError(ErrorKind::Name, "'" + node.name + "' has no value yet — store one first, e.g. " + node.name +
                                         " = 5", node.position);
}

void Evaluator::visit(const UnaryNode& node)
{
    const double operand = evaluateChild(*node.operand);

    if (node.op == UnaryOp::Negate) {
        result_ = -operand;
        // "-5" typed as a literal is just a negative number, not a step worth showing.
        if (dynamic_cast<const NumberNode*>(node.operand.get()) == nullptr) {
            record(StepKind::Operation, "−" + operandText(operand), result_, "Negation flips the sign");
        }
        return;
    }

    double value = 0.0;
    try {
        value = factorial(operand);
    } catch (const CalcError& error) {
        throw CalcError(error.kind(), error.what(), node.position);
    }
    record(StepKind::Operation, operandText(operand) + "!", value, explainFactorial(operand));
    result_ = value;
}

void Evaluator::visit(const BinaryNode& node)
{
    const double left = evaluateChild(*node.left);
    const double right = evaluateChild(*node.right);

    double value = 0.0;
    switch (node.op) {
    case BinaryOp::Add: value = left + right; break;
    case BinaryOp::Subtract: value = left - right; break;
    case BinaryOp::Multiply: value = left * right; break;
    case BinaryOp::Divide:
        if (right == 0.0) {
            throw CalcError(ErrorKind::Math,
                            left == 0.0 ? "0 ÷ 0 is indeterminate — any number times 0 gives 0"
                                        : "Division by zero is undefined — no number times 0 gives " +
                                              formatNumber(left),
                            node.position);
        }
        value = left / right;
        break;
    case BinaryOp::Power:
        if (left == 0.0 && right < 0) {
            throw CalcError(ErrorKind::Math, "0 to a negative power would mean dividing by zero", node.position);
        }
        if (left < 0 && !isWhole(right)) {
            throw CalcError(ErrorKind::Math,
                            "A negative number to a fractional power has no real answer — try root(x, n)",
                            node.position);
        }
        value = std::pow(left, right);
        break;
    }

    value = ensureFinite(value, node);
    record(StepKind::Operation, operandText(left) + " " + std::string(symbol(node.op)) + " " + operandText(right),
           value, explainBinary(node, left, right));
    result_ = value;
}

void Evaluator::visit(const CallNode& node)
{
    const FunctionInfo* function = library_.findFunction(node.name);
    if (function == nullptr) {
        if (library_.findConstant(node.name) != nullptr || environment_.variable(node.name)) {
            throw CalcError(ErrorKind::Name, "'" + node.name + "' is a value, not a function — write " + node.name +
                                                 " × (…) to multiply", node.position);
        }
        throw CalcError(ErrorKind::Name, "Unknown function '" + node.name + "' — see the Reference tab",
                        node.position);
    }
    if (node.arguments.size() != function->arity) {
        throw CalcError(ErrorKind::Syntax,
                        function->name + " takes " + std::to_string(function->arity) +
                            (function->arity == 1 ? " argument" : " arguments") + " but got " +
                            std::to_string(node.arguments.size()) + " — usage: " + function->signature,
                        node.position);
    }

    std::vector<double> arguments;
    arguments.reserve(node.arguments.size());
    for (const NodePtr& argument : node.arguments) arguments.push_back(evaluateChild(*argument));

    const bool degrees = environment_.angleMode() == AngleMode::Degrees;
    std::vector<double> inputs = arguments;
    if (degrees && function->angleRole == AngleRole::TakesAngle) {
        for (double& angle : inputs) angle = angle * kPi / 180.0;
    }

    double value = 0.0;
    try {
        value = function->apply(inputs);
    } catch (const CalcError& error) {
        throw CalcError(error.kind(), error.what(), node.position);
    }
    if (degrees && function->angleRole == AngleRole::ReturnsAngle) value = value * 180.0 / kPi;
    value = ensureFinite(value, node);

    std::string call = function->name + "(";
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) call += ", ";
        call += formatNumber(arguments[i]);
        if (degrees && function->angleRole == AngleRole::TakesAngle) call += "°";
    }
    call += ")";

    std::string explanation = function->description;
    if (function->angleRole == AngleRole::TakesAngle) explanation += degrees ? " (angle in degrees)" : " (angle in radians)";
    if (function->angleRole == AngleRole::ReturnsAngle) explanation += degrees ? " (answer in degrees)" : " (answer in radians)";

    record(StepKind::Function, call, value, explanation);
    result_ = value;
}

void Evaluator::visit(const AssignNode& node)
{
    if (library_.findConstant(node.name) != nullptr) {
        throw CalcError(ErrorKind::Name, "'" + node.name + "' is a built-in constant and cannot be changed",
                        node.position);
    }
    if (library_.findFunction(node.name) != nullptr) {
        throw CalcError(ErrorKind::Name, "'" + node.name + "' is a function name — pick another variable name",
                        node.position);
    }

    const double value = evaluateChild(*node.value);
    environment_.setVariable(node.name, value);
    record(StepKind::Assignment, node.name, value,
           "Stored in the variable '" + node.name + "' — use it in later expressions");
    result_ = value;
}

}  // namespace calc::core
