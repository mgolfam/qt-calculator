#pragma once

#include "application/Evaluation.h"

#include <QFrame>

class QLabel;
class QLineEdit;

namespace calc::ui {

/// The expression input plus the large result line underneath it.
class DisplayWidget : public QFrame {
    Q_OBJECT

public:
    explicit DisplayWidget(QWidget* parent = nullptr);

    QString expression() const;
    void setExpression(const QString& text);
    /// Inserts at the cursor. Function names ("sin(") get their closing bracket and
    /// wrap any selected text.
    void insert(const QString& text);
    void backspace();
    void clear();
    void selectAll();
    void focusExpression();

    void showPreview(const application::Evaluation& evaluation);
    void showCommitted(const application::Evaluation& evaluation);

signals:
    void expressionChanged(const QString& text);
    void submitted(const QString& text);
    /// -1 = older entry (↑), +1 = newer entry (↓).
    void historyNavigation(int direction);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void showIdle();
    void setResult(const QString& text, const QString& state);
    void setMessage(const QString& text, const QString& state);

    QLineEdit* edit_ = nullptr;
    QLabel* result_ = nullptr;
    QLabel* message_ = nullptr;
};

}  // namespace calc::ui
