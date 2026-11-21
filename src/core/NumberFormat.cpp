#include "core/NumberFormat.h"

#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace calc::core {

std::string formatNumber(double value, int significantDigits)
{
    if (std::isnan(value)) return "undefined";
    if (std::isinf(value)) return value > 0 ? "∞" : "−∞";
    if (value == 0.0) return "0";  // also turns -0 into 0

    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(significantDigits) << std::abs(value);
    return (value < 0 ? "−" : "") + out.str();
}

}  // namespace calc::core
