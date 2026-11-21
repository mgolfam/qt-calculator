#include "ui/panels/TreePanel.h"

#include "ui/Theme.h"
#include "ui/widgets/AstDiagramWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace calc::ui {

TreePanel::TreePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* intro = new QLabel(tr("<b>Stage 2 · Parsing.</b> The tokens are arranged into a tree. Operators higher "
                                "up are done <i>later</i>, so precedence is built into the shape. The calculator "
                                "solves it from the leaves upward — the purple numbers match the Steps tab."),
                             this);
    intro->setProperty("role", QStringLiteral("intro"));
    intro->setWordWrap(true);

    auto* legend = new QHBoxLayout;
    legend->setSpacing(16);
    const std::pair<core::OutlineKind, QString> entries[] = {
        {core::OutlineKind::Number, tr("number")},
        {core::OutlineKind::Variable, tr("variable / constant")},
        {core::OutlineKind::Operator, tr("operator")},
        {core::OutlineKind::Function, tr("function")},
        {core::OutlineKind::Assignment, tr("assignment")},
    };
    for (const auto& [kind, name] : entries) {
        auto* label = new QLabel(QStringLiteral("<span style='color:%1; font-size:16px'>●</span> %2")
                                     .arg(theme::colorFor(kind).name(), name),
                                 this);
        legend->addWidget(label);
    }
    legend->addStretch();

    diagram_ = new AstDiagramWidget;
    auto* scroll = new QScrollArea(this);
    scroll->setObjectName(QStringLiteral("TreeScroll"));
    scroll->setWidget(diagram_);
    scroll->setWidgetResizable(true);
    scroll->viewport()->setAutoFillBackground(false);
    diagram_->setAutoFillBackground(false);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);
    layout->addWidget(intro);
    layout->addLayout(legend);
    layout->addWidget(scroll, 1);
}

void TreePanel::showEvaluation(const application::Evaluation& evaluation) { diagram_->setTree(evaluation.tree); }

}  // namespace calc::ui
