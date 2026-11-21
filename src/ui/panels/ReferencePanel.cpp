#include "ui/panels/ReferencePanel.h"

#include "core/NumberFormat.h"
#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace calc::ui {

ReferencePanel::ReferencePanel(const core::MathLibrary& library, QWidget* parent)
    : QWidget(parent)
{
    auto* intro = new QLabel(tr("Everything the calculator understands. <b>Double-click</b> a function or "
                                "constant to insert it."),
                             this);
    intro->setProperty("role", QStringLiteral("intro"));
    intro->setWordWrap(true);

    search_ = new QLineEdit(this);
    search_->setPlaceholderText(tr("Search, e.g. root, angle, log…"));
    search_->setClearButtonEnabled(true);

    tree_ = new QTreeWidget(this);
    tree_->setColumnCount(3);
    tree_->setHeaderLabels({tr("Name"), tr("What it does"), tr("Example")});
    tree_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tree_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    tree_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tree_->setIndentation(14);
    tree_->setUniformRowHeights(true);

    QTreeWidgetItem* order = addCategory(tr("Order of operations (first → last)"));
    addEntry(order, QStringLiteral("( )"), tr("Brackets — the innermost group is solved first"),
             QStringLiteral("(2 + 3) × 4 = 20"), {});
    addEntry(order, QStringLiteral("f(x)"), tr("Function calls"), QStringLiteral("sqrt(9) + 1 = 4"), {});
    addEntry(order, QStringLiteral("n!"), tr("Factorial"), QStringLiteral("3! × 2 = 12"), {});
    addEntry(order, QStringLiteral("^"), tr("Powers — right to left!"), QStringLiteral("2^3^2 = 2^9 = 512"), {});
    addEntry(order, QStringLiteral("−x"), tr("Negation — after powers"), QStringLiteral("−2^2 = −4"), {});
    addEntry(order, QStringLiteral("× ÷  2x"), tr("Multiply, divide, implicit × — left to right"),
             QStringLiteral("8 ÷ 4 × 2 = 4"), {});
    addEntry(order, QStringLiteral("+ −"), tr("Add, subtract — left to right"), QStringLiteral("10 − 3 + 2 = 9"), {});
    addEntry(order, QStringLiteral("="), tr("Store a value in a variable"), QStringLiteral("r = 3"), {});

    QTreeWidgetItem* constants = addCategory(tr("Constants"));
    for (const core::ConstantInfo& constant : library.constants()) {
        addEntry(constants, toQString(constant.symbol + "  (" + constant.name + ")"), toQString(constant.description),
                 toQString(core::formatNumber(constant.value)), toQString(constant.name));
    }

    std::vector<std::string> categoryOrder;
    for (const core::FunctionInfo& function : library.functions()) {
        if (std::find(categoryOrder.begin(), categoryOrder.end(), function.category) == categoryOrder.end()) {
            categoryOrder.push_back(function.category);
        }
    }
    for (const std::string& categoryName : categoryOrder) {
        QTreeWidgetItem* category = addCategory(toQString(categoryName));
        for (const core::FunctionInfo& function : library.functions()) {
            if (function.category != categoryName) continue;
            addEntry(category, toQString(function.signature), toQString(function.description),
                     toQString(function.example), toQString(function.name + "("));
        }
    }
    tree_->expandAll();

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);
    layout->addWidget(intro);
    layout->addWidget(search_);
    layout->addWidget(tree_, 1);

    connect(search_, &QLineEdit::textChanged, this, &ReferencePanel::filter);
    connect(tree_, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item) {
        const QString text = item->data(0, Qt::UserRole).toString();
        if (!text.isEmpty()) emit insertRequested(text);
    });
}

QTreeWidgetItem* ReferencePanel::addCategory(const QString& title)
{
    auto* item = new QTreeWidgetItem(tree_, {title});
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
    item->setForeground(0, theme::kAccentAlt);
    item->setFirstColumnSpanned(true);
    return item;
}

void ReferencePanel::addEntry(QTreeWidgetItem* category, const QString& name, const QString& description,
                              const QString& example, const QString& insertText)
{
    auto* item = new QTreeWidgetItem(category, {name, description, example});
    item->setFont(0, theme::monospaceFont(11));
    item->setFont(2, theme::monospaceFont(11));
    item->setForeground(0, insertText.isEmpty() ? theme::colorFor(core::OutlineKind::Operator)
                                                : theme::colorFor(core::OutlineKind::Function));
    item->setForeground(2, theme::kTextMuted);
    item->setData(0, Qt::UserRole, insertText);
    if (!insertText.isEmpty()) item->setToolTip(0, tr("Double-click to insert"));
}

void ReferencePanel::filter(const QString& text)
{
    const QString needle = text.trimmed();
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        QTreeWidgetItem* category = tree_->topLevelItem(i);
        int visible = 0;
        for (int j = 0; j < category->childCount(); ++j) {
            QTreeWidgetItem* entry = category->child(j);
            bool matches = needle.isEmpty();
            for (int column = 0; column < 3 && !matches; ++column) {
                matches = entry->text(column).contains(needle, Qt::CaseInsensitive);
            }
            entry->setHidden(!matches);
            visible += matches ? 1 : 0;
        }
        category->setHidden(visible == 0);
    }
}

}  // namespace calc::ui
