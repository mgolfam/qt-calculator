#include "ui/panels/TokensPanel.h"

#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

namespace calc::ui {
namespace {

QColor colorFor(core::TokenType type)
{
    switch (type) {
    case core::TokenType::Number: return theme::colorFor(core::OutlineKind::Number);
    case core::TokenType::Identifier: return theme::colorFor(core::OutlineKind::Variable);
    case core::TokenType::Equals: return theme::colorFor(core::OutlineKind::Assignment);
    case core::TokenType::LeftParen:
    case core::TokenType::RightParen:
    case core::TokenType::Comma:
    case core::TokenType::End: return theme::kTextMuted;
    default: return theme::colorFor(core::OutlineKind::Operator);
    }
}

QLabel* makeCodeLabel(QWidget* parent)
{
    auto* label = new QLabel(parent);
    label->setProperty("role", QStringLiteral("code"));
    label->setFont(theme::monospaceFont(13));
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    return label;
}

}  // namespace

TokensPanel::TokensPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* intro = new QLabel(tr("<b>Stage 1 · Lexing.</b> Before anything is calculated, the text is cut into "
                                "<i>tokens</i> — the words of the math language. Spaces are thrown away; "
                                "<code>π</code> becomes the name <code>pi</code>."),
                             this);
    intro->setProperty("role", QStringLiteral("intro"));
    intro->setWordWrap(true);

    table_ = new QTableWidget(0, 3, this);
    table_->setHorizontalHeaderLabels({tr("Token"), tr("Kind"), tr("Position")});
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setShowGrid(false);
    table_->setFont(theme::monospaceFont(12));

    infix_ = makeCodeLabel(this);
    postfix_ = makeCodeLabel(this);

    auto* forms = new QFormLayout;
    forms->setLabelAlignment(Qt::AlignRight);
    auto* infixTitle = new QLabel(tr("Fully bracketed"), this);
    infixTitle->setToolTip(tr("The same expression with every operation in brackets — this is the order of operations"));
    auto* postfixTitle = new QLabel(tr("Postfix (RPN)"), this);
    postfixTitle->setToolTip(tr("Reverse Polish notation: operators come after their operands. Read left to "
                                "right with a stack and you never need brackets."));
    forms->addRow(infixTitle, infix_);
    forms->addRow(postfixTitle, postfix_);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);
    layout->addWidget(intro);
    layout->addWidget(table_, 1);
    layout->addLayout(forms);
}

void TokensPanel::showEvaluation(const application::Evaluation& evaluation)
{
    table_->setRowCount(0);
    for (const core::Token& token : evaluation.tokens) {
        if (token.type == core::TokenType::End) continue;
        const int row = table_->rowCount();
        table_->insertRow(row);

        auto* text = new QTableWidgetItem(toQString(token.text));
        text->setForeground(colorFor(token.type));
        auto* kind = new QTableWidgetItem(toQString(std::string(core::describe(token.type))));
        auto* position = new QTableWidgetItem(QString::number(token.position));
        position->setForeground(theme::kTextMuted);

        table_->setItem(row, 0, text);
        table_->setItem(row, 1, kind);
        table_->setItem(row, 2, position);
    }

    const QString none = QStringLiteral("—");
    infix_->setText(evaluation.infix.empty() ? none : toQString(evaluation.infix));
    postfix_->setText(evaluation.postfix.empty() ? none : toQString(evaluation.postfix));
}

}  // namespace calc::ui
