#include "TestHelpers.h"

#include "application/CalculatorService.h"

#include <QtTest>

using namespace calc;
using application::CalculatorService;
using application::HistoryEntry;
using application::InMemoryHistoryRepository;

namespace {

/// Test double that records how often the service saved.
class CountingRepository final : public application::HistoryRepository {
public:
    explicit CountingRepository(int& saves) : saves_(saves) {}
    std::vector<HistoryEntry> load() override { return {{"1 + 1", "2"}}; }
    void save(const std::vector<HistoryEntry>&) override { ++saves_; }

private:
    int& saves_;
};

}  // namespace

class CalculatorServiceTest : public QObject {
    Q_OBJECT

private slots:
    void previewDoesNotChangeState()
    {
        CalculatorService service(std::make_unique<InMemoryHistoryRepository>());
        const auto evaluation = service.preview("x = 5");
        QVERIFY(evaluation.success);
        QCOMPARE(evaluation.assignedVariable, std::optional<std::string>("x"));
        QVERIFY(service.variables().count("x") == 0);
        QVERIFY(service.history().empty());
    }

    void submitUpdatesAnswerHistoryAndVariables()
    {
        CalculatorService service(std::make_unique<InMemoryHistoryRepository>());
        QVERIFY(service.submit("r = 3").success);
        QVERIFY(service.submit("  π r^2  ").success);

        QCOMPARE(service.history().size(), std::size_t{2});
        QCOMPARE(test::q(service.history().back().expression), QStringLiteral("π r^2"));
        QCOMPARE(test::q(service.history().back().result), QStringLiteral("28.2743338823"));
        QCOMPARE(service.submit("ans / pi").value, 9.0);
    }

    void failedSubmitIsNotRecorded()
    {
        int saves = 0;
        CalculatorService service(std::make_unique<CountingRepository>(saves));
        QCOMPARE(service.history().size(), std::size_t{1});  // loaded from the repository

        const auto evaluation = service.submit("1 / 0");
        QVERIFY(!evaluation.success);
        QCOMPARE(evaluation.errorKind, std::optional<core::ErrorKind>(core::ErrorKind::Math));
        QCOMPARE(evaluation.errorPosition, std::optional<std::size_t>(2));
        QCOMPARE(service.history().size(), std::size_t{1});
        QCOMPARE(saves, 0);

        QVERIFY(service.submit("2 + 2").success);
        QCOMPARE(saves, 1);
    }

    void partialArtifactsSurviveErrors()
    {
        CalculatorService service(nullptr);
        const auto evaluation = service.preview("(2 + 3) / (1 - 1)");
        QVERIFY(!evaluation.success);
        QVERIFY(!evaluation.tokens.empty());
        QVERIFY(evaluation.tree.has_value());
        QCOMPARE(evaluation.steps.size(), std::size_t{2});  // 2 + 3 and 1 − 1 ran before ÷ failed
    }

    void emptyInput()
    {
        CalculatorService service(nullptr);
        const auto evaluation = service.preview("   ");
        QVERIFY(evaluation.empty);
        QVERIFY(!evaluation.success);
        QVERIFY(evaluation.error.empty());
    }

    void clearVariablesKeepsAnswer()
    {
        CalculatorService service(nullptr);
        service.submit("a = 7");
        service.clearVariables();
        QVERIFY(service.variables().count("a") == 0);
        QCOMPARE(service.lastAnswer(), 7.0);
    }

    void historyIsCapped()
    {
        CalculatorService service(nullptr);
        for (std::size_t i = 0; i < CalculatorService::kMaxHistory + 10; ++i) service.submit("1");
        QCOMPARE(service.history().size(), CalculatorService::kMaxHistory);
    }

    void plotting()
    {
        CalculatorService service(nullptr);
        service.submit("a = 2");

        const auto parabola = service.compileForPlot("a x^2");
        QVERIFY(parabola.isValid());
        QCOMPARE(parabola(3.0), std::optional<double>(18.0));

        const auto root = service.compileForPlot("sqrt(x)");
        QVERIFY(root.isValid());
        QCOMPARE(root(-1.0), std::optional<double>());

        // Graphs use radians even when the calculator is in degrees.
        QCOMPARE(service.angleMode(), core::AngleMode::Degrees);
        QVERIFY(std::abs(*service.compileForPlot("sin(x)")(3.14159265358979 / 2) - 1.0) < 1e-12);

        QVERIFY(!service.compileForPlot("z + 1").isValid());
        QVERIFY(!service.compileForPlot("x +").isValid());
    }
};

QTEST_APPLESS_MAIN(CalculatorServiceTest)
#include "tst_calculator_service.moc"
