#include "ui/panels/GraphPanel.h"

#include "ui/Strings.h"
#include "ui/Theme.h"
#include "ui/widgets/GraphWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace calc::ui {

GraphPanel::GraphPanel(const application::CalculatorService& service, QWidget* parent)
    : QWidget(parent)
    , service_(service)
{
    input_ = new QLineEdit(QStringLiteral("sin(x); x^2/4"), this);
    input_->setPlaceholderText(tr("Functions of x, separated by ;   e.g.  sin(x); x^2/4; 2x + 1"));
    input_->setFont(theme::monospaceFont(13));

    auto* plotButton = new QPushButton(tr("Plot"), this);
    plotButton->setProperty("role", QStringLiteral("accent"));
    auto* fromCalculator = new QPushButton(tr("Use calculator input"), this);
    fromCalculator->setToolTip(tr("Plot whatever is typed in the calculator display"));
    auto* reset = new QPushButton(tr("Reset view"), this);

    auto* controls = new QHBoxLayout;
    controls->setSpacing(8);
    controls->addWidget(new QLabel(QStringLiteral("y ="), this));
    controls->addWidget(input_, 1);
    controls->addWidget(plotButton);
    controls->addWidget(fromCalculator);
    controls->addWidget(reset);

    graph_ = new GraphWidget(this);

    status_ = new QLabel(this);
    status_->setObjectName(QStringLiteral("Hint"));
    status_->setWordWrap(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);
    layout->addLayout(controls);
    layout->addWidget(graph_, 1);
    layout->addWidget(status_);

    connect(input_, &QLineEdit::returnPressed, this, &GraphPanel::replot);
    connect(plotButton, &QPushButton::clicked, this, &GraphPanel::replot);
    connect(fromCalculator, &QPushButton::clicked, this, &GraphPanel::useCalculatorExpressionRequested);
    connect(reset, &QPushButton::clicked, graph_, &GraphWidget::resetView);

    replot();
}

void GraphPanel::plotExpression(const QString& expression)
{
    if (expression.trimmed().isEmpty()) return;
    input_->setText(expression);
    replot();
}

void GraphPanel::replot()
{
    std::vector<Curve> curves;
    QStringList problems;

    int index = 0;
    for (const QString& part : input_->text().split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
        const QString text = part.trimmed();
        if (text.isEmpty()) continue;

        const application::PlotFunction function = service_.compileForPlot(toStdString(text));
        if (!function.isValid()) {
            problems << tr("“%1”: %2").arg(text, toQString(function.error()));
            continue;
        }
        curves.push_back(Curve{text, theme::curveColor(index++), [function](double x) { return function(x); }});
    }
    graph_->setCurves(std::move(curves));

    if (problems.isEmpty()) {
        status_->setText(tr("Scroll to zoom · drag to pan · double-click to reset · hover to read values · "
                            "graphs always use radians · your variables work too (a = 2, then a x^2)"));
        theme::setStyleState(status_, "state", QString());
    } else {
        status_->setText(problems.join(QLatin1Char('\n')));
        theme::setStyleState(status_, "state", QStringLiteral("error"));
    }
}

}  // namespace calc::ui
