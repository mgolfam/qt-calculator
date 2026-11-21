#pragma once

#include "application/Evaluation.h"

#include <QWidget>

class QTextBrowser;

namespace calc::ui {

/// "Show your work": bracketed form, numbered steps with explanations, the answer.
class StepsPanel : public QWidget {
    Q_OBJECT

public:
    explicit StepsPanel(QWidget* parent = nullptr);

    void showEvaluation(const application::Evaluation& evaluation);

signals:
    /// The user clicked one of the example expressions.
    void exampleRequested(const QString& expression);

private:
    QTextBrowser* browser_ = nullptr;
};

}  // namespace calc::ui
