#include "ui/panels/BaseConverterPanel.h"

#include "core/BaseConverter.h"
#include "core/Errors.h"
#include "core/NumberFormat.h"
#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <cmath>

namespace calc::ui {
namespace {

/// "10011100" → "1001 1100": one group of four bits is exactly one hex digit.
QString grouped(const std::string& digits, int size)
{
    QString text = toQString(digits);
    const int signLength = text.startsWith(QLatin1Char('-')) ? 1 : 0;
    for (int i = text.size() - size; i > signLength; i -= size) text.insert(i, QLatin1Char(' '));
    return text;
}

QString stepsHtml(const std::vector<std::string>& steps)
{
    QString html = QStringLiteral("<table cellpadding='3'>");
    for (const std::string& step : steps) {
        html += QStringLiteral("<tr><td class='step'>%1</td></tr>").arg(toQString(step).toHtmlEscaped());
    }
    return html + QStringLiteral("</table>");
}

QSpinBox* makeBaseSpin(int value, QWidget* parent)
{
    auto* spin = new QSpinBox(parent);
    spin->setRange(core::BaseConverter::kMinBase, core::BaseConverter::kMaxBase);
    spin->setValue(value);
    spin->setMinimumWidth(72);
    return spin;
}

QLabel* addBaseCard(QGridLayout* grid, int column, const QString& title, QWidget* parent)
{
    auto* card = new QFrame(parent);
    card->setObjectName(QStringLiteral("BaseCard"));
    auto* caption = new QLabel(title, card);
    caption->setProperty("role", QStringLiteral("baseTitle"));
    auto* value = new QLabel(card);
    value->setProperty("role", QStringLiteral("baseValue"));
    value->setFont(theme::monospaceFont(12));
    value->setTextInteractionFlags(Qt::TextSelectableByMouse);
    value->setWordWrap(true);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->addWidget(caption);
    layout->addWidget(value);
    grid->addWidget(card, 0, column);
    return value;
}

}  // namespace

BaseConverterPanel::BaseConverterPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* intro = new QLabel(tr("<b>Number bases.</b> Every base uses the same idea: each position is worth "
                                "<i>base</i> times more than the one to its right. Computers think in base 2; "
                                "programmers write base 16 because one hex digit = four bits."),
                             this);
    intro->setProperty("role", QStringLiteral("intro"));
    intro->setWordWrap(true);

    input_ = new QLineEdit(QStringLiteral("156"), this);
    input_->setFont(theme::monospaceFont(13));
    input_->setPlaceholderText(tr("Whole number, e.g. 156, FF or 0b1010"));
    fromBase_ = makeBaseSpin(10, this);
    toBase_ = makeBaseSpin(2, this);
    auto* useAnswer = new QPushButton(tr("Use last answer"), this);

    auto* controls = new QHBoxLayout;
    controls->setSpacing(8);
    controls->addWidget(input_, 1);
    controls->addWidget(new QLabel(tr("from base"), this));
    controls->addWidget(fromBase_);
    controls->addWidget(new QLabel(tr("to base"), this));
    controls->addWidget(toBase_);
    controls->addWidget(useAnswer);

    result_ = new QLabel(this);
    result_->setObjectName(QStringLiteral("Result"));
    result_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    result_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    auto* cards = new QGridLayout;
    cards->setSpacing(8);
    binary_ = addBaseCard(cards, 0, tr("BINARY · 2"), this);
    octal_ = addBaseCard(cards, 1, tr("OCTAL · 8"), this);
    decimal_ = addBaseCard(cards, 2, tr("DECIMAL · 10"), this);
    hex_ = addBaseCard(cards, 3, tr("HEX · 16"), this);
    cards->setColumnStretch(0, 2);
    for (int column = 1; column < 4; ++column) cards->setColumnStretch(column, 1);

    steps_ = new QTextBrowser(this);
    steps_->document()->setDefaultStyleSheet(
        QStringLiteral("h3 { color: #E6E9F2; font-size: 14px; margin-top: 10px; }"
                       ".step { font-family: '%1'; font-size: 14px; color: #E6E9F2; }"
                       ".muted { color: #8A93A8; } .error { color: #F87171; font-weight: bold; }")
            .arg(theme::monospaceFont(12).family()));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);
    layout->addWidget(intro);
    layout->addLayout(controls);
    layout->addWidget(result_);
    layout->addLayout(cards);
    layout->addWidget(steps_, 1);

    connect(input_, &QLineEdit::textChanged, this, [this] { convert(); });
    connect(fromBase_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { convert(); });
    connect(toBase_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { convert(); });
    connect(useAnswer, &QPushButton::clicked, this, &BaseConverterPanel::useAnswerRequested);

    convert();
}

void BaseConverterPanel::setDecimalValue(double value)
{
    const QString shown = toQString(core::formatNumber(value));
    if (!std::isfinite(value) || std::abs(value) > 9.2e18) {
        showError(tr("The last answer (%1) is too large to convert — only whole numbers up to about "
                     "9.2 × 10¹⁸ (2⁶³ − 1) fit.")
                      .arg(shown));
        return;
    }

    const long long whole = std::llround(value);
    {
        // Set both inputs first, then convert once.
        const QSignalBlocker blockBase(fromBase_);
        const QSignalBlocker blockInput(input_);
        fromBase_->setValue(10);
        input_->setText(QString::number(whole));
    }
    convert(static_cast<double>(whole) == value
                ? QString()
                : tr("The last answer %1 is not a whole number, so it was rounded to %2.").arg(shown).arg(whole));
}

void BaseConverterPanel::showError(const QString& message)
{
    result_->setText(QStringLiteral("—"));
    for (QLabel* label : {binary_, octal_, decimal_, hex_}) label->setText(QStringLiteral("—"));
    steps_->setHtml(QStringLiteral("<p class='error'>%1</p>").arg(message.toHtmlEscaped()));
}

void BaseConverterPanel::convert(const QString& notice)
{
    const int from = fromBase_->value();
    const int to = toBase_->value();

    try {
        const core::DecimalConversion decimal = core::BaseConverter::toDecimal(toStdString(input_->text()), from);
        const core::BaseConversion target = core::BaseConverter::fromDecimal(decimal.value, to);

        result_->setText(QStringLiteral("%1<sub style='font-size:14px'> %2</sub>")
                             .arg(toQString(target.digits).toHtmlEscaped())
                             .arg(to));
        binary_->setText(grouped(core::BaseConverter::fromDecimal(decimal.value, 2).digits, 4));
        octal_->setText(toQString(core::BaseConverter::fromDecimal(decimal.value, 8).digits));
        decimal_->setText(QString::number(decimal.value));
        hex_->setText(toQString(core::BaseConverter::fromDecimal(decimal.value, 16).digits));

        QString html;
        if (!notice.isEmpty()) html += QStringLiteral("<p class='muted'>%1</p>").arg(notice.toHtmlEscaped());
        if (from != 10) {
            html += tr("<h3>1 · Base %1 → decimal (add up place values)</h3>").arg(from) + stepsHtml(decimal.steps);
        }
        if (to != 10) {
            html += tr("<h3>%1 · Decimal → base %2 (divide repeatedly, keep the remainders)</h3>")
                        .arg(from != 10 ? 2 : 1)
                        .arg(to) +
                    stepsHtml(target.steps);
        }
        if (from == 10 && to == 10) {
            html += tr("<p class='muted'>Both bases are 10 — nothing to convert. Pick another base above.</p>");
        }
        steps_->setHtml(html);
    } catch (const core::CalcError& error) {
        showError(toQString(error.what()));
    }
}

}  // namespace calc::ui
