#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace calc::core {

struct NumberNode;
struct VariableNode;
struct UnaryNode;
struct BinaryNode;
struct CallNode;
struct AssignNode;

/// Visitor over the abstract syntax tree. Evaluation, printing and the tree
/// outline are all separate visitors, so the node types stay plain data.
class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;
    virtual void visit(const NumberNode& node) = 0;
    virtual void visit(const VariableNode& node) = 0;
    virtual void visit(const UnaryNode& node) = 0;
    virtual void visit(const BinaryNode& node) = 0;
    virtual void visit(const CallNode& node) = 0;
    virtual void visit(const AssignNode& node) = 0;
};

/// Base of every node in the abstract syntax tree (AST).
struct Node {
    explicit Node(std::size_t sourcePosition) : position(sourcePosition) {}
    virtual ~Node() = default;
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    virtual void accept(NodeVisitor& visitor) const = 0;

    std::size_t position;  ///< Where the node starts in the source text.
};

using NodePtr = std::unique_ptr<Node>;

enum class UnaryOp { Negate, Factorial };
enum class BinaryOp { Add, Subtract, Multiply, Divide, Power };

std::string_view symbol(UnaryOp op);
std::string_view symbol(BinaryOp op);

/// A literal such as `42` or `6.02e23`.
struct NumberNode final : Node {
    NumberNode(double literal, std::string source, std::size_t pos)
        : Node(pos), value(literal), text(std::move(source)) {}
    void accept(NodeVisitor& visitor) const override { visitor.visit(*this); }

    double value;
    std::string text;
};

/// A name that refers to a variable or a constant, such as `x` or `pi`.
struct VariableNode final : Node {
    VariableNode(std::string identifier, std::size_t pos) : Node(pos), name(std::move(identifier)) {}
    void accept(NodeVisitor& visitor) const override { visitor.visit(*this); }

    std::string name;
};

/// `-a` or `a!`.
struct UnaryNode final : Node {
    UnaryNode(UnaryOp operation, NodePtr child, std::size_t pos)
        : Node(pos), op(operation), operand(std::move(child)) {}
    void accept(NodeVisitor& visitor) const override { visitor.visit(*this); }

    UnaryOp op;
    NodePtr operand;
};

/// `a + b`, `a × b`, `a ^ b`, ... `implicit` marks multiplication written as `2x`.
struct BinaryNode final : Node {
    BinaryNode(BinaryOp operation, NodePtr lhs, NodePtr rhs, std::size_t pos, bool isImplicit = false)
        : Node(pos), op(operation), left(std::move(lhs)), right(std::move(rhs)), implicit(isImplicit) {}
    void accept(NodeVisitor& visitor) const override { visitor.visit(*this); }

    BinaryOp op;
    NodePtr left;
    NodePtr right;
    bool implicit;
};

/// A function call such as `sin(x)` or `max(a, b)`.
struct CallNode final : Node {
    CallNode(std::string function, std::vector<NodePtr> args, std::size_t pos)
        : Node(pos), name(std::move(function)), arguments(std::move(args)) {}
    void accept(NodeVisitor& visitor) const override { visitor.visit(*this); }

    std::string name;
    std::vector<NodePtr> arguments;
};

/// `name = expression` — stores a value in a variable.
struct AssignNode final : Node {
    AssignNode(std::string variable, NodePtr expression, std::size_t pos)
        : Node(pos), name(std::move(variable)), value(std::move(expression)) {}
    void accept(NodeVisitor& visitor) const override { visitor.visit(*this); }

    std::string name;
    NodePtr value;
};

}  // namespace calc::core
