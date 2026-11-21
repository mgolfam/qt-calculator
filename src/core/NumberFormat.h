#pragma once

#include <string>

namespace calc::core {

/// Formats a value for display: up to `significantDigits` digits, trailing zeros
/// removed, typographic minus sign, and words for non-finite values.
std::string formatNumber(double value, int significantDigits = 12);

}  // namespace calc::core
