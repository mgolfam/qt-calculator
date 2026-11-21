#pragma once

#include "application/Evaluation.h"

#include <QWidget>

class QLabel;
class QTableWidget;

namespace calc::ui {

/// Stage 1 (lexing): the token table, plus the bracketed and postfix forms.
class TokensPanel : public QWidget {
    Q_OBJECT

public:
    explicit TokensPanel(QWidget* parent = nullptr);

    void showEvaluation(const application::Evaluation& evaluation);

private:
    QTableWidget* table_ = nullptr;
    QLabel* infix_ = nullptr;
    QLabel* postfix_ = nullptr;
};

}  // namespace calc::ui
