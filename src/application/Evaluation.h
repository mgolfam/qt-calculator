#pragma once

#include "core/AstPrinter.h"
#include "core/Errors.h"
#include "core/Evaluator.h"
#include "core/Token.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace calc::application {

/// Everything the UI needs to show about one evaluation — the answer *and* the
/// intermediate artefacts of each stage, for the learning panels.
///
/// Stages fill in as far as they get: a parse error still has tokens, and a math
/// error still has tokens, the tree and the steps leading up to the failure.
struct Evaluation {
    std::string input;
    bool empty = true;
    bool success = false;

    double value = 0.0;
    std::string formattedValue;
    std::optional<std::string> assignedVariable;

    std::vector<core::Token> tokens;           // stage 1: lexing
    std::string infix;                         // stage 2: parsing
    std::string postfix;
    std::optional<core::AstOutline> tree;
    std::vector<core::EvaluationStep> steps;   // stage 3: evaluation

    std::string error;
    std::optional<core::ErrorKind> errorKind;
    std::optional<std::size_t> errorPosition;
};

}  // namespace calc::application
