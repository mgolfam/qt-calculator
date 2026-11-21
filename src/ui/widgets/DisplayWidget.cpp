#include "ui/widgets/DisplayWidget.h"

#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace calc::ui {

DisplayWidget::DisplayWidget(QWidget* parent)
    : QFrame(parent)
{
    edit_ = new QLineEdit(this);
    edit_->setObjectName(QStringLiteral("Expression"));
    edit_->setPlaceholderText(tr("Type an expression, e.g. 2 + 3 × 4"));
    edit_->setFont(theme::monospaceFont(17));
    edit_->setClearButtonEnabled(true);
    edit_->installEventFilter(this);

    result_ = new QLabel(this);
    result_->setObjectName(QStringLiteral("Result"));
    result_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    result_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    result_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    result_->setMinimumHeight(56);

    message_ = new QLabel(this);
    message_->setObjectName(QStringLiteral("Message"));
    message_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    message_->setWordWrap(true);
    message_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    message_->setMinimumHeight(36);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(edit_);
    layout->addWidget(result_);
    layout->addWidget(message_);

    connect(edit_, &QLineEdit::textChanged, this, &DisplayWidget::expressionChanged);
    connect(edit_, &QLineEdit::returnPressed, this, [this] { emit submitted(edit_->text()); });

    showIdle();
}

QString DisplayWidget::expression() const { return edit_->text(); }

void DisplayWidget::setExpression(const QString& text)
{
    edit_->setText(text);
    edit_->setFocus();
}

void DisplayWidget::insert(const QString& text)
{
    const bool isFunction = text.size() > 1 && text.endsWith(QLatin1Char('('));
    if (isFunction) {
        const QString selected = edit_->selectedText();
        edit_->insert(text + selected + QLatin1Char(')'));
        if (selected.isEmpty()) edit_->cursorBackward(false);
    } else {
        edit_->insert(text);
    }
    edit_->setFocus();
}

void DisplayWidget::backspace()
{
    edit_->backspace();
    edit_->setFocus();
}

void DisplayWidget::clear()
{
    edit_->clear();
    edit_->setFocus();
}

void DisplayWidget::selectAll() { edit_->selectAll(); }

void DisplayWidget::focusExpression() { edit_->setFocus(); }

void DisplayWidget::showPreview(const application::Evaluation& evaluation)
{
    if (evaluation.empty) {
        showIdle();
        return;
    }
    if (evaluation.success) {
        setResult(QStringLiteral("= ") + toQString(evaluation.formattedValue), QStringLiteral("preview"));
        setMessage(evaluation.assignedVariable
                       ? tr("Press Enter to store it in %1").arg(toQString(*evaluation.assignedVariable))
                       : tr("Press Enter to calculate"),
                   QStringLiteral("hint"));
    } else {
        setResult(QStringLiteral("…"), QStringLiteral("preview"));
        setMessage(toQString(evaluation.error), QStringLiteral("warning"));
    }
}

void DisplayWidget::showCommitted(const application::Evaluation& evaluation)
{
    if (evaluation.empty) return;

    if (evaluation.success) {
        const QString value = toQString(evaluation.formattedValue);
        setResult(QStringLiteral("= ") + value, QStringLiteral("final"));
        setMessage(evaluation.assignedVariable
                       ? tr("Stored  %1 = %2").arg(toQString(*evaluation.assignedVariable), value)
                       : tr("Saved to history  ·  ans = %1").arg(value),
                   QStringLiteral("success"));
        return;
    }

    const QString title = evaluation.errorKind ? toQString(std::string(core::describe(*evaluation.errorKind)))
                                               : tr("Error");
    setResult(title, QStringLiteral("error"));
    setMessage(toQString(evaluation.error), QStringLiteral("error"));

    if (evaluation.errorPosition) {
        // Highlight the character where the calculator got confused.
        const int position = static_cast<int>(*evaluation.errorPosition);
        edit_->setFocus();
        if (position < edit_->text().size()) {
            edit_->setSelection(position, 1);
        } else {
            edit_->setCursorPosition(edit_->text().size());
        }
    }
}

bool DisplayWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == edit_ && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent*>(event)->key()) {
        case Qt::Key_Up:
            emit historyNavigation(-1);
            return true;
        case Qt::Key_Down:
            emit historyNavigation(+1);
            return true;
        case Qt::Key_Escape:
            edit_->clear();
            return true;
        default:
            break;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void DisplayWidget::showIdle()
{
    setResult(QStringLiteral("0"), QStringLiteral("preview"));
    setMessage(tr("Every step is explained in the tabs on the right →"), QStringLiteral("hint"));
}

void DisplayWidget::setResult(const QString& text, const QString& state)
{
    result_->setText(text);
    result_->setToolTip(text);
    theme::setStyleState(result_, "state", state);
}

void DisplayWidget::setMessage(const QString& text, const QString& state)
{
    message_->setText(text);
    theme::setStyleState(message_, "state", state);
}

}  // namespace calc::ui
