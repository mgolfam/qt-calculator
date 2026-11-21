#pragma once

#include "core/MathLibrary.h"

#include <QWidget>

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

namespace calc::ui {

/// Searchable cheat-sheet: order of operations, constants and every function.
class ReferencePanel : public QWidget {
    Q_OBJECT

public:
    explicit ReferencePanel(const core::MathLibrary& library, QWidget* parent = nullptr);

signals:
    void insertRequested(const QString& text);

private:
    QTreeWidgetItem* addCategory(const QString& title);
    void addEntry(QTreeWidgetItem* category, const QString& name, const QString& description,
                  const QString& example, const QString& insertText);
    void filter(const QString& text);

    QLineEdit* search_ = nullptr;
    QTreeWidget* tree_ = nullptr;
};

}  // namespace calc::ui
