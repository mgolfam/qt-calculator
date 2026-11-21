#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace calc::core {

/// A number written in some base, plus the worked steps that produced it.
struct BaseConversion {
    std::string digits;
    std::vector<std::string> steps;
};

/// A decimal value, plus the positional-expansion steps that produced it.
struct DecimalConversion {
    long long value = 0;
    std::vector<std::string> steps;
};

/// Converts whole numbers between bases 2…36 and explains how.
class BaseConverter {
public:
    static constexpr int kMinBase = 2;
    static constexpr int kMaxBase = 36;

    /// Repeated division: 13 → 1101 in base 2.
    static BaseConversion fromDecimal(long long value, int base);

    /// Positional expansion: 1101 in base 2 → 1×2³ + 1×2² + 0×2¹ + 1×2⁰ = 13.
    /// Accepts an optional sign and the prefixes 0b / 0o / 0x for matching bases.
    /// @throws CalcError on invalid digits or overflow.
    static DecimalConversion toDecimal(std::string_view digits, int base);
};

}  // namespace calc::core
