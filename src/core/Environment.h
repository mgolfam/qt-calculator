#pragma once

#include <map>
#include <optional>
#include <string>

namespace calc::core {

enum class AngleMode { Radians, Degrees };

/// Mutable evaluation state: user variables and the angle unit.
class Environment {
public:
    void setVariable(const std::string& name, double value) { variables_[name] = value; }

    std::optional<double> variable(const std::string& name) const
    {
        const auto it = variables_.find(name);
        if (it == variables_.end()) return std::nullopt;
        return it->second;
    }

    const std::map<std::string, double>& variables() const noexcept { return variables_; }
    void clearVariables() { variables_.clear(); }

    AngleMode angleMode() const noexcept { return angleMode_; }
    void setAngleMode(AngleMode mode) noexcept { angleMode_ = mode; }

private:
    std::map<std::string, double> variables_;
    AngleMode angleMode_ = AngleMode::Degrees;  // what most school courses start with
};

}  // namespace calc::core
