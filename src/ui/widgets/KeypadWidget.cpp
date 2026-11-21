#include "ui/widgets/KeypadWidget.h"

#include "ui/Strings.h"

#include <QGridLayout>
#include <QPushButton>
#include <QVariant>

namespace calc::ui {
namespace {

enum class KeyAction { Insert, Clear, Backspace, Evaluate };

struct KeySpec {
    const char* label;
    const char* insert;
    const char* role;
    const char* tooltip;  // nullptr: taken from the math library
    int row;
    int column;
    int columnSpan;
    KeyAction action;
};

constexpr KeyAction Insert = KeyAction::Insert;

// clang-format off
const KeySpec kKeys[] = {
    {"sin",  "sin(",  "function", nullptr, 0, 0, 1, Insert},
    {"cos",  "cos(",  "function", nullptr, 0, 1, 1, Insert},
    {"tan",  "tan(",  "function", nullptr, 0, 2, 1, Insert},
    {"ln",   "ln(",   "function", nullptr, 0, 3, 1, Insert},
    {"log",  "log(",  "function", nullptr, 0, 4, 1, Insert},
    {"√",    "sqrt(", "function", nullptr, 0, 5, 1, Insert},

    {"asin", "asin(", "function", nullptr, 1, 0, 1, Insert},
    {"acos", "acos(", "function", nullptr, 1, 1, 1, Insert},
    {"atan", "atan(", "function", nullptr, 1, 2, 1, Insert},
    {"x²",   "^2",    "operator", "Square: a number times itself", 1, 3, 1, Insert},
    {"xʸ",   "^",     "operator", "Power: 2^3 = 2 × 2 × 2", 1, 4, 1, Insert},
    {"n!",   "!",     "operator", "Factorial: 5! = 5 × 4 × 3 × 2 × 1", 1, 5, 1, Insert},

    {"(",    "(",     "paren",    "Open a group — groups are calculated first", 2, 0, 1, Insert},
    {")",    ")",     "paren",    "Close a group", 2, 1, 1, Insert},
    {"π",    "π",     "constant", "π ≈ 3.14159 — a circle's circumference ÷ its diameter", 2, 2, 1, Insert},
    {"e",    "e",     "constant", "e ≈ 2.71828 — Euler's number", 2, 3, 1, Insert},
    {"x",    "x",     "variable", "The variable x — store a value (x = 3) or plot it in the Graph tab", 2, 4, 1, Insert},
    {"ans",  "ans",   "variable", "The previous answer", 2, 5, 1, Insert},

    {"7",    "7",     "digit",    "", 3, 0, 1, Insert},
    {"8",    "8",     "digit",    "", 3, 1, 1, Insert},
    {"9",    "9",     "digit",    "", 3, 2, 1, Insert},
    {"÷",    "÷",     "operator", "Divide  ( / )", 3, 3, 1, Insert},
    {"AC",   "",      "danger",   "Clear everything  (Esc)", 3, 4, 1, KeyAction::Clear},
    {"⌫",    "",      "danger",   "Delete one character  (Backspace)", 3, 5, 1, KeyAction::Backspace},

    {"4",    "4",     "digit",    "", 4, 0, 1, Insert},
    {"5",    "5",     "digit",    "", 4, 1, 1, Insert},
    {"6",    "6",     "digit",    "", 4, 2, 1, Insert},
    {"×",    "×",     "operator", "Multiply  ( * )", 4, 3, 1, Insert},
    {"mod",  "mod(",  "function", nullptr, 4, 4, 1, Insert},
    {"abs",  "abs(",  "function", nullptr, 4, 5, 1, Insert},

    {"1",    "1",     "digit",    "", 5, 0, 1, Insert},
    {"2",    "2",     "digit",    "", 5, 1, 1, Insert},
    {"3",    "3",     "digit",    "", 5, 2, 1, Insert},
    {"−",    "−",     "operator", "Subtract, or make a number negative  ( - )", 5, 3, 1, Insert},
    {"nCr",  "nCr(",  "function", nullptr, 5, 4, 1, Insert},
    {",",    ", ",    "paren",    "Separates function arguments, e.g. max(3, 7)", 5, 5, 1, Insert},

    {"0",    "0",     "digit",    "", 6, 0, 2, Insert},
    {".",    ".",     "digit",    "Decimal point", 6, 2, 1, Insert},
    {"+",    "+",     "operator", "Add", 6, 3, 1, Insert},
    {"=",    "",      "equals",   "Calculate  (Enter)", 6, 4, 2, KeyAction::Evaluate},
};
// clang-format on

QString tooltipFor(const KeySpec& key, const core::MathLibrary& library)
{
    if (key.tooltip != nullptr) return QString::fromUtf8(key.tooltip);

    std::string name = key.insert;
    if (!name.empty() && name.back() == '(') name.pop_back();
    if (const core::FunctionInfo* function = library.findFunction(name)) {
        return toQString(function->signature + "\n" + function->description + "\nExample: " + function->example);
    }
    return {};
}

}  // namespace

KeypadWidget::KeypadWidget(const core::MathLibrary& library, QWidget* parent)
    : QWidget(parent)
{
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(7);

    for (const KeySpec& key : kKeys) {
        auto* button = new QPushButton(QString::fromUtf8(key.label), this);
        button->setProperty("role", QString::fromUtf8(key.role));
        button->setToolTip(tooltipFor(key, library));
        button->setFocusPolicy(Qt::NoFocus);  // keep the cursor in the expression
        button->setCursor(Qt::PointingHandCursor);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setMinimumSize(52, 38);
        grid->addWidget(button, key.row, key.column, 1, key.columnSpan);

        switch (key.action) {
        case KeyAction::Insert: {
            const QString text = QString::fromUtf8(key.insert);
            connect(button, &QPushButton::clicked, this, [this, text] { emit insertRequested(text); });
            break;
        }
        case KeyAction::Clear:
            connect(button, &QPushButton::clicked, this, &KeypadWidget::clearRequested);
            break;
        case KeyAction::Backspace:
            connect(button, &QPushButton::clicked, this, &KeypadWidget::backspaceRequested);
            break;
        case KeyAction::Evaluate:
            connect(button, &QPushButton::clicked, this, &KeypadWidget::evaluateRequested);
            break;
        }
    }
}

}  // namespace calc::ui
