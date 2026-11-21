#include "ui/panels/StepsPanel.h"

#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

#include <iterator>

namespace calc::ui {
namespace {

const char* const kExamples[] = {
    "2 + 3 × 4",       "(2 + 3) × 4",    "2^3^2",       "−2^2",          "sin(30)^2 + cos(30)^2",
    "r = 3",           "π r^2",          "5!",          "nCr(5, 2)",     "7 ÷ 2",
    "sqrt(2)^2",       "1 ÷ 0",
};

QString escaped(const std::string& text) { return toQString(text).toHtmlEscaped(); }

QString stepsStyleSheet()
{
    const QString mono = theme::monospaceFont(12).family();
    return QStringLiteral(
               "h3 { color: #E6E9F2; font-size: 15px; margin-top: 14px; margin-bottom: 4px; }"
               ".muted { color: #8A93A8; }"
               ".code { font-family: '%1'; font-size: 17px; color: #22D3EE; }"
               ".step { font-family: '%1'; font-size: 15px; color: #E6E9F2; }"
               ".badge { color: #7C5CFF; font-weight: bold; font-size: 15px; }"
               ".answer { font-family: '%1'; font-size: 30px; font-weight: bold; color: #34D399; }"
               ".error { color: #F87171; font-weight: bold; font-size: 16px; }"
               "a { color: #22D3EE; text-decoration: none; }")
        .arg(mono);
}

QString welcomeHtml()
{
    QString examples;
    for (std::size_t i = 0; i < std::size(kExamples); ++i) {
        if (i > 0) examples += QStringLiteral(" &nbsp;·&nbsp; ");
        examples += QStringLiteral("<a href='example:%1'>%2</a>")
                        .arg(i)
                        .arg(QString::fromUtf8(kExamples[i]).toHtmlEscaped());
    }

    return QStringLiteral(
               "<h3>How a calculator thinks</h3>"
               "<p class='muted'>Every expression goes through three stages. Each tab shows one of them:</p>"
               "<table cellpadding='5'>"
               "<tr><td class='badge'>1</td><td><b>Tokens</b> <span class='muted'>— the text is cut into "
               "numbers, names and operators.</span></td></tr>"
               "<tr><td class='badge'>2</td><td><b>Tree</b> <span class='muted'>— the tokens are arranged into a "
               "tree that captures the order of operations.</span></td></tr>"
               "<tr><td class='badge'>3</td><td><b>Steps</b> <span class='muted'>— the tree is solved from the "
               "leaves up, one small calculation at a time.</span></td></tr>"
               "</table>"
               "<h3>Try one</h3><p class='code'>%1</p>"
               "<p class='muted'>Tip: store values with <b>r = 3</b> (press Enter), then reuse them. "
               "Writing <b>2x</b> or <b>3(4 + 1)</b> means multiplication.</p>")
        .arg(examples);
}

QString errorTip(core::ErrorKind kind)
{
    switch (kind) {
    case core::ErrorKind::Syntax:
        return QObject::tr("The expression isn't written in a way the calculator understands. Look for a missing "
                           "number, operator or bracket around the highlighted spot.");
    case core::ErrorKind::Math:
        return QObject::tr("The expression is written correctly, but the math has no real answer.");
    case core::ErrorKind::Name:
        return QObject::tr("The Reference tab lists every function and constant. Your own variables appear in "
                           "the History tab.");
    }
    return {};
}

}  // namespace

StepsPanel::StepsPanel(QWidget* parent)
    : QWidget(parent)
{
    browser_ = new QTextBrowser(this);
    browser_->setOpenLinks(false);
    browser_->document()->setDefaultStyleSheet(stepsStyleSheet());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->addWidget(browser_);

    connect(browser_, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        bool ok = false;
        const int index = url.path().toInt(&ok);
        if (url.scheme() == QLatin1String("example") && ok && index >= 0 &&
            static_cast<std::size_t>(index) < std::size(kExamples)) {
            emit exampleRequested(QString::fromUtf8(kExamples[index]));
        }
    });

    showEvaluation({});
}

void StepsPanel::showEvaluation(const application::Evaluation& evaluation)
{
    if (evaluation.empty) {
        browser_->setHtml(welcomeHtml());
        return;
    }

    QString html;
    if (!evaluation.infix.empty()) {
        html += tr("<h3>Order of operations</h3><p class='code'>%1</p>"
                   "<p class='muted'>Brackets show the order the calculator follows — the innermost "
                   "brackets are worked out first.</p>")
                    .arg(escaped(evaluation.infix));
    }

    if (!evaluation.steps.empty()) {
        html += tr("<h3>Step by step</h3>");
        html += QStringLiteral("<table cellpadding='5' cellspacing='0' width='100%'>");
        int number = 0;
        for (const core::EvaluationStep& step : evaluation.steps) {
            const QString arrow = step.kind == core::StepKind::Assignment ? QStringLiteral(" ← ")
                                                                           : QStringLiteral(" = ");
            html += QStringLiteral("<tr><td width='38' valign='top' class='badge' style='white-space:nowrap'>%1</td>"
                                   "<td><span class='step'>%2%3<b>%4</b></span><br>"
                                   "<span class='muted'>%5</span></td></tr>")
                        .arg(++number)
                        .arg(escaped(step.expression), arrow, escaped(step.result), escaped(step.explanation));
        }
        html += QStringLiteral("</table>");
    } else if (evaluation.success) {
        html += tr("<h3>Step by step</h3><p class='muted'>A single number — nothing to calculate.</p>");
    }

    if (evaluation.success) {
        html += tr("<h3>Answer</h3><p class='answer'>%1</p>").arg(escaped(evaluation.formattedValue));
    } else {
        const QString title = evaluation.errorKind ? toQString(std::string(core::describe(*evaluation.errorKind)))
                                                   : tr("Error");
        html += QStringLiteral("<h3>%1</h3><p class='error'>%2</p><p class='muted'>%3</p>")
                    .arg(title.toHtmlEscaped(), escaped(evaluation.error),
                         evaluation.errorKind ? errorTip(*evaluation.errorKind).toHtmlEscaped() : QString());
    }

    browser_->setHtml(html);
}

}  // namespace calc::ui
