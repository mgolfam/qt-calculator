#include "core/BaseConverter.h"

#include "core/Errors.h"

#include <climits>

namespace calc::core {
namespace {

constexpr std::string_view kDigits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void validateBase(int base)
{
    if (base < BaseConverter::kMinBase || base > BaseConverter::kMaxBase) {
        throw CalcError(ErrorKind::Math, "Bases must be between 2 and 36");
    }
}

int digitValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    return -1;
}

std::string allowedDigits(int base)
{
    if (base <= 10) return "0–" + std::string(1, kDigits[static_cast<std::size_t>(base - 1)]);
    return "0–9 and A–" + std::string(1, kDigits[static_cast<std::size_t>(base - 1)]);
}

std::string superscript(int n)
{
    static const char* const kSuperscripts[] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};
    std::string out;
    for (const char c : std::to_string(n)) out += kSuperscripts[c - '0'];
    return out;
}

std::string_view trim(std::string_view text)
{
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.remove_prefix(1);
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) text.remove_suffix(1);
    return text;
}

std::string_view stripPrefix(std::string_view text, int base)
{
    if (text.size() < 2 || text[0] != '0') return text;
    const char marker = static_cast<char>(text[1] | 0x20);  // lower-case
    if ((base == 16 && marker == 'x') || (base == 2 && marker == 'b') || (base == 8 && marker == 'o')) {
        text.remove_prefix(2);
    }
    return text;
}

}  // namespace

BaseConversion BaseConverter::fromDecimal(long long value, int base)
{
    validateBase(base);
    BaseConversion result;

    if (value == 0) {
        result.digits = "0";
        result.steps.push_back("0 is written as 0 in every base");
        return result;
    }

    const bool negative = value < 0;
    unsigned long long n = negative ? 0ULL - static_cast<unsigned long long>(value) : static_cast<unsigned long long>(value);
    const auto divisor = static_cast<unsigned long long>(base);

    if (negative) {
        result.steps.push_back("The number is negative: convert " + std::to_string(n) +
                               " and put a minus sign in front");
    }

    std::string reversed;
    while (n > 0) {
        const unsigned long long quotient = n / divisor;
        const unsigned long long remainder = n % divisor;
        const char digit = kDigits[remainder];
        std::string line = std::to_string(n) + " ÷ " + std::to_string(base) + " = " + std::to_string(quotient) +
                           " remainder " + std::to_string(remainder);
        if (remainder >= 10) line += "  → digit " + std::string(1, digit);
        result.steps.push_back(std::move(line));
        reversed.push_back(digit);
        n = quotient;
    }

    result.digits = (negative ? "-" : "") + std::string(reversed.rbegin(), reversed.rend());
    result.steps.push_back("Read the remainders from bottom to top: " + result.digits);
    return result;
}

DecimalConversion BaseConverter::toDecimal(std::string_view input, int base)
{
    validateBase(base);
    std::string_view text = trim(input);

    bool negative = false;
    if (!text.empty() && (text.front() == '-' || text.front() == '+')) {
        negative = text.front() == '-';
        text.remove_prefix(1);
    }
    text = stripPrefix(text, base);
    if (text.empty()) throw CalcError(ErrorKind::Syntax, "Enter a number to convert");

    std::size_t firstSignificant = 0;
    while (firstSignificant + 1 < text.size() && text[firstSignificant] == '0') ++firstSignificant;
    text.remove_prefix(firstSignificant);

    std::vector<int> digits;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const int value = digitValue(text[i]);
        if (value < 0 || value >= base) {
            throw CalcError(ErrorKind::Syntax, "'" + std::string(1, text[i]) + "' is not a digit in base " +
                                                   std::to_string(base) + " — allowed: " + allowedDigits(base),
                            i);
        }
        digits.push_back(value);
    }

    DecimalConversion result;
    unsigned long long total = 0;
    const unsigned long long limit = static_cast<unsigned long long>(LLONG_MAX);
    for (const int digit : digits) {
        if (total > (limit - static_cast<unsigned long long>(digit)) / static_cast<unsigned long long>(base)) {
            throw CalcError(ErrorKind::Math, "That number is too large (the limit is 2⁶³ − 1)");
        }
        total = total * static_cast<unsigned long long>(base) + static_cast<unsigned long long>(digit);
    }

    // Explain: each digit is worth digit × base^position. Place values are computed
    // right-to-left; they never overflow because they are bounded by `total`.
    std::vector<unsigned long long> placeValues(digits.size(), 1);
    for (std::size_t i = digits.size() - 1; i-- > 0;) {
        placeValues[i] = placeValues[i + 1] * static_cast<unsigned long long>(base);
    }

    std::string sum;
    for (std::size_t i = 0; i < digits.size(); ++i) {
        const int power = static_cast<int>(digits.size() - 1 - i);
        const std::string term = std::to_string(static_cast<unsigned long long>(digits[i]) * placeValues[i]);
        result.steps.push_back(std::string(1, kDigits[static_cast<std::size_t>(digits[i])]) + " × " +
                               std::to_string(base) + superscript(power) + " = " + std::to_string(digits[i]) +
                               " × " + std::to_string(placeValues[i]) + " = " + term);
        if (i > 0) sum += " + ";
        sum += term;
    }
    result.steps.push_back("Add the place values: " + sum + " = " + std::to_string(total));

    result.value = negative ? -static_cast<long long>(total) : static_cast<long long>(total);
    if (negative) result.steps.push_back("Apply the minus sign: " + std::to_string(result.value));
    return result;
}

}  // namespace calc::core
