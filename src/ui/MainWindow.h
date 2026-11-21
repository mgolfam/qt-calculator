#pragma once

#include "application/CalculatorService.h"

#include <QMainWindow>

class QPushButton;
class QTabWidget;

namespace calc::ui {

class BaseConverterPanel;
class DisplayWidget;
class GraphPanel;
class HistoryPanel;
class KeypadWidget;
class ReferencePanel;
class StepsPanel;
class TokensPanel;
class TreePanel;

/// Composes the widgets and routes user intents to the CalculatorService.
/// Holds no math logic of its own.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(application::CalculatorService& service, QWidget* parent = nullptr);

private:
    QWidget* buildHeader();
    QWidget* buildCalculatorCard();
    QWidget* buildLearningTabs();
    void buildMenus();
    void startTips();
    void connectSignals();

    void onExpressionChanged(const QString& text);
    void onSubmitted(const QString& text);
    void onKeypadInsert(const QString& text);
    void onHistoryNavigation(int direction);
    void onAngleModeToggled(bool degrees);
    void showExpression(const QString& expression);

    void refreshLearningPanels(const application::Evaluation& evaluation);
    void refreshHistoryPanel();

    application::CalculatorService& service_;

    DisplayWidget* display_ = nullptr;
    KeypadWidget* keypad_ = nullptr;
    QPushButton* angleToggle_ = nullptr;
    QTabWidget* tabs_ = nullptr;
    StepsPanel* steps_ = nullptr;
    TokensPanel* tokens_ = nullptr;
    TreePanel* tree_ = nullptr;
    GraphPanel* graph_ = nullptr;
    BaseConverterPanel* bases_ = nullptr;
    ReferencePanel* reference_ = nullptr;
    HistoryPanel* history_ = nullptr;

    bool justSubmitted_ = false;
    bool navigatingHistory_ = false;
    int historyCursor_ = -1;
};

}  // namespace calc::ui
