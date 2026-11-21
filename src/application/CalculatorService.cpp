#include "application/CalculatorService.h"

#include "core/AstPrinter.h"
#include "core/Errors.h"
#include "core/Evaluator.h"
#include "core/Lexer.h"
#include "core/NumberFormat.h"
#include "core/Parser.h"

#include <optional>

namespace calc::application {
namespace {

constexpr const char* kAnswerVariable = "ans";

std::string trimmed(const std::string& text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

}  // namespace

CalculatorService::CalculatorService(std::unique_ptr<HistoryRepository> repository, const core::MathLibrary& library)
    : repository_(std::move(repository))
    , library_(library)
{
    if (!repository_) repository_ = std::make_unique<InMemoryHistoryRepository>();
    history_ = repository_->load();
    if (history_.size() > kMaxHistory) {
        history_.erase(history_.begin(), history_.end() - static_cast<std::ptrdiff_t>(kMaxHistory));
    }
    environment_.setVariable(kAnswerVariable, 0.0);
}

Evaluation CalculatorService::preview(const std::string& input) const
{
    core::Environment scratch = environment_;
    return run(input, scratch);
}

Evaluation CalculatorService::submit(const std::string& input)
{
    Evaluation evaluation = run(input, environment_);
    if (!evaluation.success) return evaluation;

    environment_.setVariable(kAnswerVariable, evaluation.value);
    history_.push_back(HistoryEntry{trimmed(input), evaluation.formattedValue});
    if (history_.size() > kMaxHistory) history_.erase(history_.begin());
    repository_->save(history_);
    return evaluation;
}

PlotFunction CalculatorService::compileForPlot(const std::string& input) const
{
    return PlotFunction::compile(input, environment_, library_);
}

void CalculatorService::clearHistory()
{
    history_.clear();
    repository_->save(history_);
}

double CalculatorService::lastAnswer() const { return environment_.variable(kAnswerVariable).value_or(0.0); }

void CalculatorService::clearVariables()
{
    const double answer = lastAnswer();
    environment_.clearVariables();
    environment_.setVariable(kAnswerVariable, answer);
}

Evaluation CalculatorService::run(const std::string& input, core::Environment& environment) const
{
    Evaluation evaluation;
    evaluation.input = input;
    if (trimmed(input).empty()) return evaluation;
    evaluation.empty = false;

    // Declared outside the try so that steps recorded before a failure survive.
    std::optional<core::Evaluator> evaluator;
    try {
        evaluation.tokens = core::Lexer::tokenize(input);

        core::Parser parser(evaluation.tokens);
        const core::NodePtr ast = parser.parse();
        evaluation.infix = core::toInfix(*ast);
        evaluation.postfix = core::toPostfix(*ast);
        evaluation.tree = core::outline(*ast);

        evaluator.emplace(environment, library_);
        evaluation.value = evaluator->evaluate(*ast);
        evaluation.formattedValue = core::formatNumber(evaluation.value);
        if (const auto* assignment = dynamic_cast<const core::AssignNode*>(ast.get())) {
            evaluation.assignedVariable = assignment->name;
        }
        evaluation.success = true;
    } catch (const core::CalcError& error) {
        evaluation.error = error.what();
        evaluation.errorKind = error.kind();
        evaluation.errorPosition = error.position();
    }

    if (evaluator) evaluation.steps = evaluator->steps();
    return evaluation;
}

}  // namespace calc::application
