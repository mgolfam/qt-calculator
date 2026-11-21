#pragma once

#include "application/Evaluation.h"

#include <QWidget>

namespace calc::ui {

class AstDiagramWidget;

/// Stage 2 (parsing): the syntax tree with a colour legend.
class TreePanel : public QWidget {
    Q_OBJECT

public:
    explicit TreePanel(QWidget* parent = nullptr);

    void showEvaluation(const application::Evaluation& evaluation);

private:
    AstDiagramWidget* diagram_ = nullptr;
};

}  // namespace calc::ui
