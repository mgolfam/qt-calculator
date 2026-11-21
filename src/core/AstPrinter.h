#pragma once

#include "core/Ast.h"

#include <optional>
#include <string>
#include <vector>

namespace calc::core {

/// The expression with every operation bracketed, e.g. `2 + (3 × 4)`.
/// This makes operator precedence visible.
std::string toInfix(const Node& root);

/// Reverse Polish (postfix) notation, e.g. `2 3 4 × +` — how a stack machine sees it.
std::string toPostfix(const Node& root);

enum class OutlineKind { Number, Variable, Operator, Function, Assignment };

/// A presentation-neutral copy of the AST, ready to be drawn by any UI.
struct AstOutline {
    std::string label;
    OutlineKind kind = OutlineKind::Number;
    std::string detail;
    std::vector<AstOutline> children;
    /// 1-based index of the matching entry in Evaluator::steps(), if this node produces one.
    std::optional<int> stepNumber;
};

AstOutline outline(const Node& root);

}  // namespace calc::core
