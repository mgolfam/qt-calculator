#pragma once

#include "application/HistoryRepository.h"

#include <QWidget>

#include <map>
#include <string>
#include <vector>

class QListWidget;

namespace calc::ui {

/// Past calculations and the variables currently stored.
class HistoryPanel : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void setHistory(const std::vector<application::HistoryEntry>& entries);
    void setVariables(const std::map<std::string, double>& variables);

signals:
    void recallRequested(const QString& expression);
    void insertRequested(const QString& text);
    void clearHistoryRequested();
    void clearVariablesRequested();

private:
    QListWidget* history_ = nullptr;
    QListWidget* variables_ = nullptr;
};

}  // namespace calc::ui
