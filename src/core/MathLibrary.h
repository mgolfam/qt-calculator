#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace calc::core {

/// How a function relates to angles, so the evaluator can honour DEG/RAD mode.
enum class AngleRole {
    None,
    TakesAngle,    ///< sin, cos, tan: the argument is an angle.
    ReturnsAngle,  ///< asin, acos, atan: the result is an angle.
};

struct FunctionInfo {
    std::string name;
    std::size_t arity;
    std::string category;
    std::string signature;    ///< e.g. "nCr(n, r)"
    std::string description;  ///< one-line explanation for learners
    std::string example;      ///< e.g. "nCr(5, 2) = 10"
    AngleRole angleRole;
    std::function<double(const std::vector<double>&)> apply;  ///< may throw CalcError
};

struct ConstantInfo {
    std::string name;
    std::string symbol;
    double value;
    std::string description;
};

/// The catalogue of built-in functions and constants.
class MathLibrary {
public:
    static const MathLibrary& standard();

    const FunctionInfo* findFunction(std::string_view name) const;
    const ConstantInfo* findConstant(std::string_view name) const;

    const std::vector<FunctionInfo>& functions() const noexcept { return functions_; }
    const std::vector<ConstantInfo>& constants() const noexcept { return constants_; }

private:
    MathLibrary();

    std::vector<FunctionInfo> functions_;
    std::vector<ConstantInfo> constants_;
};

/// n! for whole numbers 0…170. @throws CalcError (ErrorKind::Math) otherwise.
double factorial(double n);

}  // namespace calc::core
