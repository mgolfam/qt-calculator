#pragma once

#include "application/Evaluation.h"
#include "application/HistoryRepository.h"
#include "application/PlotFunction.h"
#include "core/Environment.h"
#include "core/MathLibrary.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace calc::application {

/// The application's use cases. The UI talks only to this class; it never touches
/// the lexer, parser or evaluator directly.
class CalculatorService {
public:
    static constexpr std::size_t kMaxHistory = 200;

    explicit CalculatorService(std::unique_ptr<HistoryRepository> repository,
                               const core::MathLibrary& library = core::MathLibrary::standard());

    /// Live evaluation while typing. Never changes state: assignments are not stored.
    Evaluation preview(const std::string& input) const;

    /// Commits an evaluation: stores assignments, updates `ans` and the history.
    Evaluation submit(const std::string& input);

    /// Compiles `input` as a function of x, using the current variables.
    PlotFunction compileForPlot(const std::string& input) const;

    core::AngleMode angleMode() const noexcept { return environment_.angleMode(); }
    void setAngleMode(core::AngleMode mode) noexcept { environment_.setAngleMode(mode); }

    const std::vector<HistoryEntry>& history() const noexcept { return history_; }
    void clearHistory();

    const std::map<std::string, double>& variables() const noexcept { return environment_.variables(); }
    double lastAnswer() const;
    /// Removes user variables but keeps `ans`.
    void clearVariables();

    const core::MathLibrary& library() const noexcept { return library_; }

private:
    Evaluation run(const std::string& input, core::Environment& environment) const;

    std::unique_ptr<HistoryRepository> repository_;
    const core::MathLibrary& library_;
    core::Environment environment_;
    std::vector<HistoryEntry> history_;
};

}  // namespace calc::application
