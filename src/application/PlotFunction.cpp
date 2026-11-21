#include "application/PlotFunction.h"

#include "core/Errors.h"
#include "core/Evaluator.h"
#include "core/Parser.h"

#include <cmath>

namespace calc::application {

PlotFunction PlotFunction::compile(const std::string& expression, core::Environment environment,
                                   const core::MathLibrary& library)
{
    PlotFunction function;
    function.expression_ = expression;
    function.library_ = &library;

    environment.setAngleMode(core::AngleMode::Radians);
    function.environment_ = std::make_shared<core::Environment>(std::move(environment));

    try {
        function.ast_ = core::parse(expression);
        // Probe once: unknown names or wrong argument counts fail for every x, so
        // report them now instead of silently drawing nothing.
        function.environment_->setVariable("x", 0.5);
        core::Evaluator probe(*function.environment_, library, false);
        probe.evaluate(*function.ast_);
    } catch (const core::CalcError& error) {
        if (error.kind() != core::ErrorKind::Math) {
            function.ast_.reset();
            function.error_ = error.what();
        }
    }
    return function;
}

std::optional<double> PlotFunction::operator()(double x) const
{
    if (!ast_) return std::nullopt;
    environment_->setVariable("x", x);
    core::Evaluator evaluator(*environment_, *library_, false);
    try {
        const double y = evaluator.evaluate(*ast_);
        if (!std::isfinite(y)) return std::nullopt;
        return y;
    } catch (const core::CalcError&) {
        return std::nullopt;
    }
}

}  // namespace calc::application
