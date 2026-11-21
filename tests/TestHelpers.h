#pragma once

#include "core/Environment.h"
#include "core/Errors.h"
#include "core/Evaluator.h"
#include "core/MathLibrary.h"
#include "core/Parser.h"

#include <QString>

#include <optional>
#include <string>

namespace calc::test {

inline QString q(const std::string& text) { return QString::fromStdString(text); }

/// Runs `action` and returns the CalcError it threw, if any.
template <typename Action>
std::optional<core::CalcError> captureError(Action&& action)
{
    try {
        action();
    } catch (const core::CalcError& error) {
        return error;
    }
    return std::nullopt;
}

inline double evaluate(const std::string& input, core::AngleMode mode = core::AngleMode::Degrees)
{
    core::Environment environment;
    environment.setAngleMode(mode);
    const core::NodePtr ast = core::parse(input);
    core::Evaluator evaluator(environment, core::MathLibrary::standard());
    return evaluator.evaluate(*ast);
}

}  // namespace calc::test
