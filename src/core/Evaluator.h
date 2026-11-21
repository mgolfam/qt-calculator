#pragma once

#include "core/Ast.h"
#include "core/Environment.h"
#include "core/MathLibrary.h"

#include <string>
#include <vector>

namespace calc::core {

enum class StepKind {
    Lookup,      ///< reading a variable or constant
    Operation,   ///< +, −, ×, ÷, ^, !, negation
    Function,    ///< sin(…), sqrt(…), …
    Assignment,  ///< x = …
};

/// One line of the "show your work" explanation, e.g. `3 × 4 = 12`.
struct EvaluationStep {
    StepKind kind;
    std::string expression;   ///< "3 × 4"
    std::string result;       ///< "12"
    std::string explanation;  ///< "Multiplication"
};

/// Stage 3 of evaluation: walks the AST bottom-up and computes the value.
///
/// Children are always evaluated before their parent, which is exactly the order a
/// person follows when respecting operator precedence — so the recorded steps double
/// as a worked solution.
class Evaluator final : private NodeVisitor {
public:
    Evaluator(Environment& environment, const MathLibrary& library, bool recordSteps = true);

    /// @throws CalcError with the position of the node that failed.
    double evaluate(const Node& root);

    /// Steps recorded so far — still valid (and useful) after a CalcError.
    const std::vector<EvaluationStep>& steps() const noexcept { return steps_; }

private:
    void visit(const NumberNode& node) override;
    void visit(const VariableNode& node) override;
    void visit(const UnaryNode& node) override;
    void visit(const BinaryNode& node) override;
    void visit(const CallNode& node) override;
    void visit(const AssignNode& node) override;

    double evaluateChild(const Node& node);
    void record(StepKind kind, std::string expression, double value, std::string explanation);

    Environment& environment_;
    const MathLibrary& library_;
    bool recordSteps_;
    std::vector<EvaluationStep> steps_;
    double result_ = 0.0;
};

}  // namespace calc::core
