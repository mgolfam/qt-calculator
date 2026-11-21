#include "core/MathLibrary.h"

#include "core/Errors.h"
#include "core/NumberFormat.h"

#include <cmath>
#include <numeric>

namespace calc::core {
namespace {

using Args = std::vector<double>;

[[noreturn]] void mathError(const std::string& message) { throw CalcError(ErrorKind::Math, message); }

bool isWhole(double v) { return std::isfinite(v) && std::floor(v) == v; }

/// sin(180°) is 1.2e-16 in floating point; learners expect 0.
double snapToZero(double v) { return std::abs(v) < 1e-15 ? 0.0 : v; }

long long requireInteger(double v, const std::string& function)
{
    if (!isWhole(v) || std::abs(v) > 1e15) {
        mathError(function + " works with whole numbers only (got " + formatNumber(v) + ")");
    }
    return static_cast<long long>(v);
}

double combinations(double n, double r)
{
    const long long nn = requireInteger(n, "nCr");
    const long long rr = requireInteger(r, "nCr");
    if (nn < 0 || rr < 0 || rr > nn) mathError("nCr(n, r) needs 0 ≤ r ≤ n");
    const long long k = std::min(rr, nn - rr);
    double result = 1.0;
    for (long long i = 1; i <= k; ++i) result = result * static_cast<double>(nn - k + i) / static_cast<double>(i);
    return std::round(result);
}

double permutations(double n, double r)
{
    const long long nn = requireInteger(n, "nPr");
    const long long rr = requireInteger(r, "nPr");
    if (nn < 0 || rr < 0 || rr > nn) mathError("nPr(n, r) needs 0 ≤ r ≤ n");
    double result = 1.0;
    for (long long i = 0; i < rr; ++i) result *= static_cast<double>(nn - i);
    return result;
}

double nthRoot(double x, double n)
{
    if (n == 0.0) mathError("The 0th root is undefined");
    if (x >= 0) return std::pow(x, 1.0 / n);
    if (isWhole(n) && static_cast<long long>(n) % 2 != 0) return -std::pow(-x, 1.0 / n);
    mathError("An even root of a negative number is not a real number");
}

}  // namespace

double factorial(double n)
{
    if (!isWhole(n) || n < 0) {
        mathError("Factorial is only defined for whole numbers 0, 1, 2, … (got " + formatNumber(n) + ")");
    }
    if (n > 170) mathError("n! for n > 170 is too large to represent");
    double result = 1.0;
    for (int i = 2; i <= static_cast<int>(n); ++i) result *= i;
    return result;
}

const MathLibrary& MathLibrary::standard()
{
    static const MathLibrary library;
    return library;
}

MathLibrary::MathLibrary()
{
    auto add = [this](std::string name, std::size_t arity, std::string category, std::string signature,
                      std::string description, std::string example, AngleRole role,
                      std::function<double(const Args&)> apply) {
        functions_.push_back(FunctionInfo{std::move(name), arity, std::move(category), std::move(signature),
                                          std::move(description), std::move(example), role, std::move(apply)});
    };

    const std::string trig = "Trigonometry";
    add("sin", 1, trig, "sin(θ)", "Sine: opposite ÷ hypotenuse in a right triangle", "sin(30) = 0.5 (DEG)",
        AngleRole::TakesAngle, [](const Args& a) { return snapToZero(std::sin(a[0])); });
    add("cos", 1, trig, "cos(θ)", "Cosine: adjacent ÷ hypotenuse in a right triangle", "cos(60) = 0.5 (DEG)",
        AngleRole::TakesAngle, [](const Args& a) { return snapToZero(std::cos(a[0])); });
    add("tan", 1, trig, "tan(θ)", "Tangent: opposite ÷ adjacent, i.e. sin ÷ cos", "tan(45) = 1 (DEG)",
        AngleRole::TakesAngle, [](const Args& a) {
            if (std::abs(std::cos(a[0])) < 1e-15) mathError("tan is undefined where cos = 0 (90°, 270°, …)");
            return snapToZero(std::tan(a[0]));
        });
    add("asin", 1, trig, "asin(x)", "Inverse sine: the angle whose sine is x", "asin(0.5) = 30 (DEG)",
        AngleRole::ReturnsAngle, [](const Args& a) {
            if (a[0] < -1 || a[0] > 1) mathError("asin only accepts values from −1 to 1");
            return std::asin(a[0]);
        });
    add("acos", 1, trig, "acos(x)", "Inverse cosine: the angle whose cosine is x", "acos(0.5) = 60 (DEG)",
        AngleRole::ReturnsAngle, [](const Args& a) {
            if (a[0] < -1 || a[0] > 1) mathError("acos only accepts values from −1 to 1");
            return std::acos(a[0]);
        });
    add("atan", 1, trig, "atan(x)", "Inverse tangent: the angle whose tangent is x", "atan(1) = 45 (DEG)",
        AngleRole::ReturnsAngle, [](const Args& a) { return std::atan(a[0]); });

    const std::string hyper = "Hyperbolic";
    add("sinh", 1, hyper, "sinh(x)", "Hyperbolic sine: (eˣ − e⁻ˣ) ÷ 2", "sinh(0) = 0", AngleRole::None,
        [](const Args& a) { return std::sinh(a[0]); });
    add("cosh", 1, hyper, "cosh(x)", "Hyperbolic cosine: (eˣ + e⁻ˣ) ÷ 2", "cosh(0) = 1", AngleRole::None,
        [](const Args& a) { return std::cosh(a[0]); });
    add("tanh", 1, hyper, "tanh(x)", "Hyperbolic tangent: sinh ÷ cosh", "tanh(0) = 0", AngleRole::None,
        [](const Args& a) { return std::tanh(a[0]); });

    const std::string roots = "Powers & roots";
    add("sqrt", 1, roots, "sqrt(x)", "Square root: the number that times itself gives x", "sqrt(16) = 4",
        AngleRole::None, [](const Args& a) {
            if (a[0] < 0) mathError("The square root of a negative number is not a real number");
            return std::sqrt(a[0]);
        });
    add("cbrt", 1, roots, "cbrt(x)", "Cube root: the number that cubed gives x", "cbrt(27) = 3", AngleRole::None,
        [](const Args& a) { return std::cbrt(a[0]); });
    add("root", 2, roots, "root(x, n)", "n-th root of x", "root(32, 5) = 2", AngleRole::None,
        [](const Args& a) { return nthRoot(a[0], a[1]); });
    add("exp", 1, roots, "exp(x)", "e raised to the power x", "exp(1) = 2.718…", AngleRole::None,
        [](const Args& a) { return std::exp(a[0]); });

    const std::string logs = "Logarithms";
    auto requirePositive = [](double v) {
        if (v <= 0) mathError("Logarithms are only defined for positive numbers");
    };
    add("ln", 1, logs, "ln(x)", "Natural log: the power of e that gives x", "ln(e) = 1", AngleRole::None,
        [requirePositive](const Args& a) { requirePositive(a[0]); return std::log(a[0]); });
    add("log", 1, logs, "log(x)", "Base-10 log: the power of 10 that gives x", "log(1000) = 3", AngleRole::None,
        [requirePositive](const Args& a) { requirePositive(a[0]); return std::log10(a[0]); });
    add("log2", 1, logs, "log2(x)", "Base-2 log: the power of 2 that gives x", "log2(8) = 3", AngleRole::None,
        [requirePositive](const Args& a) { requirePositive(a[0]); return std::log2(a[0]); });
    add("logb", 2, logs, "logb(x, b)", "Log of x in base b: the power of b that gives x", "logb(81, 3) = 4",
        AngleRole::None, [requirePositive](const Args& a) {
            requirePositive(a[0]);
            requirePositive(a[1]);
            if (a[1] == 1.0) mathError("A logarithm cannot have base 1");
            return std::log(a[0]) / std::log(a[1]);
        });

    const std::string rounding = "Rounding";
    add("abs", 1, rounding, "abs(x)", "Absolute value: distance from zero", "abs(−7) = 7", AngleRole::None,
        [](const Args& a) { return std::abs(a[0]); });
    add("round", 1, rounding, "round(x)", "Round to the nearest whole number", "round(2.5) = 3", AngleRole::None,
        [](const Args& a) { return std::round(a[0]); });
    add("floor", 1, rounding, "floor(x)", "Round down to a whole number", "floor(2.7) = 2", AngleRole::None,
        [](const Args& a) { return std::floor(a[0]); });
    add("ceil", 1, rounding, "ceil(x)", "Round up to a whole number", "ceil(2.1) = 3", AngleRole::None,
        [](const Args& a) { return std::ceil(a[0]); });
    add("min", 2, rounding, "min(a, b)", "The smaller of two numbers", "min(3, 7) = 3", AngleRole::None,
        [](const Args& a) { return std::min(a[0], a[1]); });
    add("max", 2, rounding, "max(a, b)", "The larger of two numbers", "max(3, 7) = 7", AngleRole::None,
        [](const Args& a) { return std::max(a[0], a[1]); });

    const std::string numbers = "Whole numbers";
    add("mod", 2, numbers, "mod(a, b)", "Remainder of a ÷ b (always has the sign of b)", "mod(17, 5) = 2",
        AngleRole::None, [](const Args& a) {
            if (a[1] == 0.0) mathError("mod(a, 0) is undefined — you cannot divide by zero");
            return a[0] - a[1] * std::floor(a[0] / a[1]);
        });
    add("gcd", 2, numbers, "gcd(a, b)", "Greatest common divisor", "gcd(12, 18) = 6", AngleRole::None,
        [](const Args& a) {
            return static_cast<double>(std::gcd(requireInteger(a[0], "gcd"), requireInteger(a[1], "gcd")));
        });
    add("lcm", 2, numbers, "lcm(a, b)", "Least common multiple", "lcm(4, 6) = 12", AngleRole::None,
        [](const Args& a) {
            const long long x = requireInteger(a[0], "lcm");
            const long long y = requireInteger(a[1], "lcm");
            if (x == 0 || y == 0) return 0.0;
            // std::lcm overflows long long for large inputs; finish the product in double.
            return std::abs(static_cast<double>(x / std::gcd(x, y)) * static_cast<double>(y));
        });
    add("fact", 1, numbers, "fact(n)", "Factorial: 1 × 2 × … × n (same as n!)", "fact(5) = 120", AngleRole::None,
        [](const Args& a) { return factorial(a[0]); });
    add("nCr", 2, numbers, "nCr(n, r)", "Combinations: ways to choose r items from n (order ignored)",
        "nCr(5, 2) = 10", AngleRole::None, [](const Args& a) { return combinations(a[0], a[1]); });
    add("nPr", 2, numbers, "nPr(n, r)", "Permutations: ways to arrange r items from n (order matters)",
        "nPr(5, 2) = 20", AngleRole::None, [](const Args& a) { return permutations(a[0], a[1]); });

    constants_ = {
        {"pi", "π", 3.14159265358979323846, "π: a circle's circumference ÷ its diameter"},
        {"tau", "τ", 6.28318530717958647692, "τ = 2π: one full turn in radians"},
        {"e", "e", 2.71828182845904523536, "Euler's number: the base of natural logarithms"},
        {"phi", "φ", 1.61803398874989484820, "φ: the golden ratio (1 + √5) ÷ 2"},
    };
}

const FunctionInfo* MathLibrary::findFunction(std::string_view name) const
{
    for (const FunctionInfo& function : functions_) {
        if (function.name == name) return &function;
    }
    return nullptr;
}

const ConstantInfo* MathLibrary::findConstant(std::string_view name) const
{
    for (const ConstantInfo& constant : constants_) {
        if (constant.name == name) return &constant;
    }
    return nullptr;
}

}  // namespace calc::core
