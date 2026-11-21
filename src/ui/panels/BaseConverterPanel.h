#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QSpinBox;
class QTextBrowser;

namespace calc::ui {

/// Number bases (binary, octal, hex, …) with the conversion worked out step by step.
class BaseConverterPanel : public QWidget {
    Q_OBJECT

public:
    explicit BaseConverterPanel(QWidget* parent = nullptr);

    /// Loads a value (rounded to a whole number) as a decimal input.
    void setDecimalValue(double value);

signals:
    void useAnswerRequested();

private:
    void convert();

    QLineEdit* input_ = nullptr;
    QSpinBox* fromBase_ = nullptr;
    QSpinBox* toBase_ = nullptr;
    QLabel* result_ = nullptr;
    QLabel* binary_ = nullptr;
    QLabel* octal_ = nullptr;
    QLabel* decimal_ = nullptr;
    QLabel* hex_ = nullptr;
    QTextBrowser* steps_ = nullptr;
};

}  // namespace calc::ui
