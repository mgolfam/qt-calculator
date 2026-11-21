#pragma once

#include "core/MathLibrary.h"

#include <QWidget>

namespace calc::ui {

/// The on-screen keys. Knows nothing about evaluation — it only emits intents.
class KeypadWidget : public QWidget {
    Q_OBJECT

public:
    explicit KeypadWidget(const core::MathLibrary& library, QWidget* parent = nullptr);

signals:
    void insertRequested(const QString& text);
    void clearRequested();
    void backspaceRequested();
    void evaluateRequested();
};

}  // namespace calc::ui
