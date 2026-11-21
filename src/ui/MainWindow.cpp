#include "ui/MainWindow.h"

#include "ui/Strings.h"
#include "ui/panels/BaseConverterPanel.h"
#include "ui/panels/GraphPanel.h"
#include "ui/panels/HistoryPanel.h"
#include "ui/panels/ReferencePanel.h"
#include "ui/panels/StepsPanel.h"
#include "ui/panels/TokensPanel.h"
#include "ui/panels/TreePanel.h"
#include "ui/widgets/DisplayWidget.h"
#include "ui/widgets/KeypadWidget.h"

#include <QAction>
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <iterator>

namespace calc::ui {
namespace {

const char* const kTips[] = {
    QT_TRANSLATE_NOOP("Tips", "Tip: ↑ / ↓ in the input browse your history."),
    QT_TRANSLATE_NOOP("Tips", "Tip: 2x or 3(4 + 1) means multiplication — no × needed."),
    QT_TRANSLATE_NOOP("Tips", "Tip: store values with r = 3, then use them: π r^2"),
    QT_TRANSLATE_NOOP("Tips", "Tip: switch DEG / RAD (top right) to change how angles are read."),
    QT_TRANSLATE_NOOP("Tips", "Tip: the purple numbers in the Tree tab match the Steps tab."),
    QT_TRANSLATE_NOOP("Tips", "Tip: plot several functions at once: sin(x); cos(x); x/2"),
    QT_TRANSLATE_NOOP("Tips", "Tip: ** works as a power too, just like in Python."),
    QT_TRANSLATE_NOOP("Tips", "Tip: Ctrl+1 … Ctrl+7 jump between the tabs."),
    QT_TRANSLATE_NOOP("Tips", "Tip: after pressing =, typing an operator continues from the answer."),
};

/// Characters that, typed right after a result, continue the calculation from `ans`.
const QString kContinuationOperators = QStringLiteral("+−×÷^!");

}  // namespace

MainWindow::MainWindow(application::CalculatorService& service, QWidget* parent)
    : QMainWindow(parent)
    , service_(service)
{
    setWindowTitle(tr("Learning Calculator"));
    resize(1260, 800);
    setMinimumSize(1000, 660);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(16, 12, 16, 8);
    root->setSpacing(14);
    root->addWidget(buildHeader());

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(14);
    splitter->addWidget(buildCalculatorCard());
    splitter->addWidget(buildLearningTabs());
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({440, 820});
    root->addWidget(splitter, 1);
    setCentralWidget(central);

    buildMenus();
    connectSignals();
    startTips();
    refreshHistoryPanel();
    display_->focusExpression();
}

QWidget* MainWindow::buildHeader()
{
    auto* header = new QFrame(this);
    header->setObjectName(QStringLiteral("Header"));

    auto* title = new QLabel(tr("Learning Calculator"), header);
    title->setObjectName(QStringLiteral("HeaderTitle"));
    auto* subtitle = new QLabel(tr("Type any expression and watch how it is solved — tokens, tree, steps and graph."),
                                header);
    subtitle->setObjectName(QStringLiteral("HeaderSubtitle"));

    auto* text = new QVBoxLayout;
    text->setSpacing(2);
    text->addWidget(title);
    text->addWidget(subtitle);

    angleToggle_ = new QPushButton(header);
    angleToggle_->setObjectName(QStringLiteral("AngleToggle"));
    angleToggle_->setCheckable(true);
    angleToggle_->setChecked(service_.angleMode() == core::AngleMode::Degrees);
    angleToggle_->setText(angleToggle_->isChecked() ? QStringLiteral("DEG") : QStringLiteral("RAD"));
    angleToggle_->setCursor(Qt::PointingHandCursor);
    angleToggle_->setToolTip(tr("Angle unit for sin, cos, tan and their inverses.\n"
                                "DEG: a full turn is 360°.  RAD: a full turn is 2π."));

    auto* layout = new QHBoxLayout(header);
    layout->setContentsMargins(22, 14, 22, 14);
    layout->addLayout(text, 1);
    layout->addWidget(angleToggle_, 0, Qt::AlignVCenter);
    return header;
}

QWidget* MainWindow::buildCalculatorCard()
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("Card"));
    card->setMinimumWidth(400);

    display_ = new DisplayWidget(card);
    keypad_ = new KeypadWidget(service_.library(), card);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addWidget(display_);
    layout->addWidget(keypad_, 1);
    return card;
}

QWidget* MainWindow::buildLearningTabs()
{
    tabs_ = new QTabWidget(this);
    tabs_->setDocumentMode(false);

    steps_ = new StepsPanel(tabs_);
    tokens_ = new TokensPanel(tabs_);
    tree_ = new TreePanel(tabs_);
    graph_ = new GraphPanel(service_, tabs_);
    bases_ = new BaseConverterPanel(tabs_);
    reference_ = new ReferencePanel(service_.library(), tabs_);
    history_ = new HistoryPanel(tabs_);

    tabs_->addTab(steps_, tr("Steps"));
    tabs_->addTab(tokens_, tr("Tokens"));
    tabs_->addTab(tree_, tr("Tree"));
    tabs_->addTab(graph_, tr("Graph"));
    tabs_->addTab(bases_, tr("Bases"));
    tabs_->addTab(reference_, tr("Reference"));
    tabs_->addTab(history_, tr("History"));
    return tabs_;
}

void MainWindow::buildMenus()
{
    QMenu* calculator = menuBar()->addMenu(tr("&Calculator"));
    calculator->addAction(tr("Clear expression"), display_, &DisplayWidget::clear);
    calculator->addAction(tr("Clear history"), this, [this] {
        service_.clearHistory();
        refreshHistoryPanel();
    });
    calculator->addSeparator();
    QAction* quit = calculator->addAction(tr("Quit"), qApp, &QApplication::quit);
    quit->setShortcut(QKeySequence::Quit);

    QMenu* view = menuBar()->addMenu(tr("&View"));
    for (int i = 0; i < tabs_->count(); ++i) {
        QAction* action = view->addAction(tabs_->tabText(i), this, [this, i] { tabs_->setCurrentIndex(i); });
        action->setShortcut(QKeySequence(Qt::CTRL | static_cast<Qt::Key>(Qt::Key_1 + i)));
    }

    QMenu* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(tr("About"), this, [this] {
        QMessageBox::about(this, tr("About Learning Calculator"),
                           tr("<h3>Learning Calculator %1</h3>"
                              "<p>A calculator that shows its work. Every expression is split into tokens, "
                              "parsed into a syntax tree and evaluated step by step, so you can see "
                              "<i>why</i> the answer is what it is.</p>"
                              "<p>Built with Qt %2.</p>")
                               .arg(QApplication::applicationVersion(), QString::fromLatin1(qVersion())));
    });
}

void MainWindow::startTips()
{
    auto* tipLabel = new QLabel(this);
    tipLabel->setContentsMargins(16, 2, 0, 4);
    statusBar()->addWidget(tipLabel, 1);
    statusBar()->setSizeGripEnabled(false);

    auto* timer = new QTimer(this);
    auto showNext = [tipLabel, index = 0]() mutable {
        tipLabel->setText(QCoreApplication::translate("Tips", kTips[index]));
        index = (index + 1) % static_cast<int>(std::size(kTips));
    };
    showNext();
    connect(timer, &QTimer::timeout, this, showNext);
    timer->start(12000);
}

void MainWindow::connectSignals()
{
    connect(display_, &DisplayWidget::expressionChanged, this, &MainWindow::onExpressionChanged);
    connect(display_, &DisplayWidget::submitted, this, &MainWindow::onSubmitted);
    connect(display_, &DisplayWidget::historyNavigation, this, &MainWindow::onHistoryNavigation);

    connect(keypad_, &KeypadWidget::insertRequested, this, &MainWindow::onKeypadInsert);
    connect(keypad_, &KeypadWidget::clearRequested, display_, &DisplayWidget::clear);
    connect(keypad_, &KeypadWidget::backspaceRequested, display_, &DisplayWidget::backspace);
    connect(keypad_, &KeypadWidget::evaluateRequested, this, [this] { onSubmitted(display_->expression()); });

    connect(angleToggle_, &QPushButton::toggled, this, &MainWindow::onAngleModeToggled);

    connect(steps_, &StepsPanel::exampleRequested, this, &MainWindow::showExpression);
    connect(reference_, &ReferencePanel::insertRequested, display_, &DisplayWidget::insert);
    connect(history_, &HistoryPanel::recallRequested, this, &MainWindow::showExpression);
    connect(history_, &HistoryPanel::insertRequested, display_, &DisplayWidget::insert);
    connect(history_, &HistoryPanel::clearHistoryRequested, this, [this] {
        service_.clearHistory();
        refreshHistoryPanel();
    });
    connect(history_, &HistoryPanel::clearVariablesRequested, this, [this] {
        service_.clearVariables();
        refreshHistoryPanel();
        onExpressionChanged(display_->expression());
    });
    connect(graph_, &GraphPanel::useCalculatorExpressionRequested, this,
            [this] { graph_->plotExpression(display_->expression()); });
    connect(bases_, &BaseConverterPanel::useAnswerRequested, this,
            [this] { bases_->setDecimalValue(service_.lastAnswer()); });
}

void MainWindow::onExpressionChanged(const QString& text)
{
    if (!navigatingHistory_) historyCursor_ = -1;
    justSubmitted_ = false;

    const application::Evaluation evaluation = service_.preview(toStdString(text));
    display_->showPreview(evaluation);
    refreshLearningPanels(evaluation);
}

void MainWindow::onSubmitted(const QString& text)
{
    const application::Evaluation evaluation = service_.submit(toStdString(text));
    display_->showCommitted(evaluation);
    refreshLearningPanels(evaluation);

    if (evaluation.success) {
        refreshHistoryPanel();
        historyCursor_ = -1;
        justSubmitted_ = true;
        display_->selectAll();  // typing now replaces the expression
    }
}

void MainWindow::onKeypadInsert(const QString& text)
{
    if (justSubmitted_) {
        // After "=", an operator continues from the answer; anything else starts fresh.
        const bool continues = !text.isEmpty() && kContinuationOperators.contains(text.front());
        display_->setExpression(continues ? QStringLiteral("ans") : QString());
    }
    display_->insert(text);
}

void MainWindow::onHistoryNavigation(int direction)
{
    const auto& entries = service_.history();
    if (entries.empty()) return;
    const int size = static_cast<int>(entries.size());

    if (direction < 0) {
        historyCursor_ = historyCursor_ < 0 ? size - 1 : std::max(0, historyCursor_ - 1);
    } else {
        if (historyCursor_ < 0) return;
        ++historyCursor_;
    }

    navigatingHistory_ = true;
    if (historyCursor_ >= size) {
        historyCursor_ = -1;
        display_->setExpression(QString());
    } else {
        display_->setExpression(toQString(entries[static_cast<std::size_t>(historyCursor_)].expression));
    }
    navigatingHistory_ = false;
}

void MainWindow::onAngleModeToggled(bool degrees)
{
    service_.setAngleMode(degrees ? core::AngleMode::Degrees : core::AngleMode::Radians);
    angleToggle_->setText(degrees ? QStringLiteral("DEG") : QStringLiteral("RAD"));
    statusBar()->showMessage(degrees ? tr("Angles are now in degrees (full turn = 360°)")
                                     : tr("Angles are now in radians (full turn = 2π)"),
                             4000);
    onExpressionChanged(display_->expression());
}

void MainWindow::showExpression(const QString& expression)
{
    display_->setExpression(expression);
    tabs_->setCurrentWidget(steps_);
}

void MainWindow::refreshLearningPanels(const application::Evaluation& evaluation)
{
    steps_->showEvaluation(evaluation);
    tokens_->showEvaluation(evaluation);
    tree_->showEvaluation(evaluation);
}

void MainWindow::refreshHistoryPanel()
{
    history_->setHistory(service_.history());
    history_->setVariables(service_.variables());
}

}  // namespace calc::ui
