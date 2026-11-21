#include "core/AstPrinter.h"

namespace calc::core {
namespace {

std::string displayName(const std::string& name)
{
    if (name == "pi") return "π";
    if (name == "tau") return "τ";
    if (name == "phi") return "φ";
    return name;
}

bool isNumberLiteral(const Node& node) { return dynamic_cast<const NumberNode*>(&node) != nullptr; }

class InfixPrinter final : public NodeVisitor {
public:
    /// Operations are bracketed only when they sit inside another operation.
    std::string render(const Node& node, bool insideOperator)
    {
        const bool saved = insideOperator_;
        insideOperator_ = insideOperator;
        node.accept(*this);
        insideOperator_ = saved;
        return text_;
    }

private:
    void visit(const NumberNode& node) override { text_ = node.text; }
    void visit(const VariableNode& node) override { text_ = displayName(node.name); }

    void visit(const UnaryNode& node) override
    {
        const bool inside = insideOperator_;
        const std::string operand = render(*node.operand, true);
        if (node.op == UnaryOp::Factorial) {
            text_ = operand + "!";
        } else {
            text_ = wrap("−" + operand, inside);
        }
    }

    void visit(const BinaryNode& node) override
    {
        const bool inside = insideOperator_;
        const std::string left = render(*node.left, true);
        const std::string right = render(*node.right, true);
        text_ = wrap(left + " " + std::string(symbol(node.op)) + " " + right, inside);
    }

    void visit(const CallNode& node) override
    {
        std::string out = node.name + "(";
        for (std::size_t i = 0; i < node.arguments.size(); ++i) {
            if (i > 0) out += ", ";
            out += render(*node.arguments[i], false);
        }
        text_ = out + ")";
    }

    void visit(const AssignNode& node) override { text_ = node.name + " = " + render(*node.value, false); }

    static std::string wrap(const std::string& text, bool inside) { return inside ? "(" + text + ")" : text; }

    std::string text_;
    bool insideOperator_ = false;
};

class PostfixPrinter final : public NodeVisitor {
public:
    std::string print(const Node& root)
    {
        root.accept(*this);
        std::string out;
        for (std::size_t i = 0; i < parts_.size(); ++i) {
            if (i > 0) out += ' ';
            out += parts_[i];
        }
        return out;
    }

private:
    void visit(const NumberNode& node) override { parts_.push_back(node.text); }
    void visit(const VariableNode& node) override { parts_.push_back(displayName(node.name)); }

    void visit(const UnaryNode& node) override
    {
        node.operand->accept(*this);
        parts_.emplace_back(node.op == UnaryOp::Negate ? "neg" : "!");
    }

    void visit(const BinaryNode& node) override
    {
        node.left->accept(*this);
        node.right->accept(*this);
        parts_.emplace_back(symbol(node.op));
    }

    void visit(const CallNode& node) override
    {
        for (const NodePtr& argument : node.arguments) argument->accept(*this);
        parts_.push_back(node.name);
    }

    void visit(const AssignNode& node) override
    {
        node.value->accept(*this);
        parts_.push_back("→" + node.name);
    }

    std::vector<std::string> parts_;
};

/// Builds the outline and numbers nodes in post-order — the same order in which
/// the Evaluator records its steps, so the tree and the step list line up.
class OutlineBuilder final : public NodeVisitor {
public:
    AstOutline build(const Node& node)
    {
        node.accept(*this);
        return std::move(result_);
    }

private:
    void visit(const NumberNode& node) override { result_ = AstOutline{node.text, OutlineKind::Number, "number", {}, {}}; }

    void visit(const VariableNode& node) override
    {
        result_ = AstOutline{displayName(node.name), OutlineKind::Variable, "variable", {}, ++step_};
    }

    void visit(const UnaryNode& node) override
    {
        AstOutline operand = build(*node.operand);
        const bool literalNegation = node.op == UnaryOp::Negate && isNumberLiteral(*node.operand);
        AstOutline out{std::string(symbol(node.op)), OutlineKind::Operator,
                       node.op == UnaryOp::Negate ? "negate" : "factorial", {}, {}};
        out.children.push_back(std::move(operand));
        if (!literalNegation) out.stepNumber = ++step_;
        result_ = std::move(out);
    }

    void visit(const BinaryNode& node) override
    {
        AstOutline left = build(*node.left);
        AstOutline right = build(*node.right);
        AstOutline out{std::string(symbol(node.op)), OutlineKind::Operator, node.implicit ? "implicit ×" : "operator",
                       {}, {}};
        out.children.push_back(std::move(left));
        out.children.push_back(std::move(right));
        out.stepNumber = ++step_;
        result_ = std::move(out);
    }

    void visit(const CallNode& node) override
    {
        AstOutline out{node.name, OutlineKind::Function, "function", {}, {}};
        for (const NodePtr& argument : node.arguments) out.children.push_back(build(*argument));
        out.stepNumber = ++step_;
        result_ = std::move(out);
    }

    void visit(const AssignNode& node) override
    {
        AstOutline value = build(*node.value);
        AstOutline out{"=", OutlineKind::Assignment, "store in " + node.name, {}, {}};
        out.children.push_back(AstOutline{node.name, OutlineKind::Variable, "target", {}, {}});
        out.children.push_back(std::move(value));
        out.stepNumber = ++step_;
        result_ = std::move(out);
    }

    AstOutline result_;
    int step_ = 0;
};

}  // namespace

std::string toInfix(const Node& root)
{
    InfixPrinter printer;
    return printer.render(root, false);
}

std::string toPostfix(const Node& root)
{
    PostfixPrinter printer;
    return printer.print(root);
}

AstOutline outline(const Node& root)
{
    OutlineBuilder builder;
    return builder.build(root);
}

}  // namespace calc::core
