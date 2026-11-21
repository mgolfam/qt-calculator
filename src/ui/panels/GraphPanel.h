#pragma once

#include "application/CalculatorService.h"

#include <QWidget>

class QLabel;
class QLineEdit;

namespace calc::ui {

class GraphWidget;

/// Plots one or more functions of x, separated by ';'.
class GraphPanel : public QWidget {
    Q_OBJECT

public:
    explicit GraphPanel(const application::CalculatorService& service, QWidget* parent = nullptr);

    void plotExpression(const QString& expression);

public slots:
    void replot();

signals:
    void useCalculatorExpressionRequested();

private:
    const application::CalculatorService& service_;
    QLineEdit* input_ = nullptr;
    GraphWidget* graph_ = nullptr;
    QLabel* status_ = nullptr;
};

}  // namespace calc::ui
