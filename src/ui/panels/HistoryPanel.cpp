#include "ui/panels/HistoryPanel.h"

#include "core/NumberFormat.h"
#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace calc::ui {
namespace {

QLabel* sectionTitle(const QString& text, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setProperty("role", QStringLiteral("sectionTitle"));
    return label;
}

}  // namespace

HistoryPanel::HistoryPanel(QWidget* parent)
    : QWidget(parent)
{
    history_ = new QListWidget(this);
    history_->setFont(theme::monospaceFont(12));
    history_->setToolTip(tr("Double-click to load an expression again"));

    variables_ = new QListWidget(this);
    variables_->setFont(theme::monospaceFont(12));
    variables_->setToolTip(tr("Double-click to insert the variable"));
    variables_->setMaximumHeight(170);

    auto* clearHistory = new QPushButton(tr("Clear history"), this);
    auto* clearVariables = new QPushButton(tr("Clear variables"), this);

    auto* historyHeader = new QHBoxLayout;
    historyHeader->addWidget(sectionTitle(tr("History  ·  newest first  ·  ↑ / ↓ in the input also works"), this), 1);
    historyHeader->addWidget(clearHistory);

    auto* variablesHeader = new QHBoxLayout;
    variablesHeader->addWidget(sectionTitle(tr("Variables  ·  create one with  name = value"), this), 1);
    variablesHeader->addWidget(clearVariables);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);
    layout->addLayout(historyHeader);
    layout->addWidget(history_, 1);
    layout->addSpacing(6);
    layout->addLayout(variablesHeader);
    layout->addWidget(variables_);

    connect(history_, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* item) { emit recallRequested(item->data(Qt::UserRole).toString()); });
    connect(variables_, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* item) { emit insertRequested(item->data(Qt::UserRole).toString()); });
    connect(clearHistory, &QPushButton::clicked, this, &HistoryPanel::clearHistoryRequested);
    connect(clearVariables, &QPushButton::clicked, this, &HistoryPanel::clearVariablesRequested);
}

void HistoryPanel::setHistory(const std::vector<application::HistoryEntry>& entries)
{
    history_->clear();
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        const QString expression = toQString(it->expression);
        auto* item = new QListWidgetItem(expression + QStringLiteral("\n  = ") + toQString(it->result), history_);
        item->setData(Qt::UserRole, expression);
    }
}

void HistoryPanel::setVariables(const std::map<std::string, double>& variables)
{
    variables_->clear();
    for (const auto& [name, value] : variables) {
        auto* item = new QListWidgetItem(toQString(name + " = " + core::formatNumber(value)), variables_);
        item->setData(Qt::UserRole, toQString(name));
        item->setForeground(theme::colorFor(core::OutlineKind::Variable));
    }
}

}  // namespace calc::ui
