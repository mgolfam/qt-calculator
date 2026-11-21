#pragma once

#include "core/Ast.h"
#include "core/Environment.h"
#include "core/MathLibrary.h"

#include <memory>
#include <optional>
#include <string>

namespace calc::application {

/// An expression in `x` compiled once and evaluated many times — one per pixel
/// column when plotting. Always evaluates in radians, like every graphing tool.
/// Cheap to copy: the AST and environment are shared.
class PlotFunction {
public:
    PlotFunction() = default;

    static PlotFunction compile(const std::string& expression, core::Environment environment,
                                const core::MathLibrary& library);

    bool isValid() const noexcept { return ast_ != nullptr; }
    const std::string& expression() const noexcept { return expression_; }
    const std::string& error() const noexcept { return error_; }

    /// f(x), or nothing where the function is undefined (e.g. sqrt(x) for x < 0).
    std::optional<double> operator()(double x) const;

private:
    std::string expression_;
    std::string error_;
    std::shared_ptr<const core::Node> ast_;
    std::shared_ptr<core::Environment> environment_;
    const core::MathLibrary* library_ = nullptr;
};

}  // namespace calc::application
