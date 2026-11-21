#include "TestHelpers.h"

#include "core/NumberFormat.h"

#include <QtTest>

using namespace calc;

class EvaluatorTest : public QObject {
    Q_OBJECT

private slots:
    void values_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<double>("expected");

        QTest::newRow("precedence") << "2 + 3 × 4" << 14.0;
        QTest::newRow("brackets") << "(2 + 3) × 4" << 20.0;
        QTest::newRow("power chain") << "2^3^2" << 512.0;
        QTest::newRow("negated power") << "-2^2" << -4.0;
        QTest::newRow("factorial") << "5!" << 120.0;
        QTest::newRow("implicit") << "2(3 + 4)" << 14.0;
        QTest::newRow("sin degrees") << "sin(30)" << 0.5;
        QTest::newRow("pythagorean identity") << "sin(37)^2 + cos(37)^2" << 1.0;
        QTest::newRow("asin degrees") << "asin(1)" << 90.0;
        QTest::newRow("sqrt") << "sqrt(16)" << 4.0;
        QTest::newRow("odd root of negative") << "root(-8, 3)" << -2.0;
        QTest::newRow("log") << "log(1000)" << 3.0;
        QTest::newRow("logb") << "logb(81, 3)" << 4.0;
        QTest::newRow("nCr") << "nCr(5, 2)" << 10.0;
        QTest::newRow("nPr") << "nPr(5, 2)" << 20.0;
        QTest::newRow("math mod") << "mod(-7, 3)" << 2.0;
        QTest::newRow("gcd") << "gcd(12, 18)" << 6.0;
        QTest::newRow("python power") << "2**10" << 1024.0;
    }

    void values()
    {
        QFETCH(QString, input);
        QFETCH(double, expected);
        QCOMPARE(test::evaluate(input.toStdString()), expected);
    }

    void sinOf180DegreesIsExactlyZero() { QCOMPARE(test::evaluate("sin(180)") + 1.0, 1.0); }

    void radians() { QCOMPARE(test::evaluate("cos(pi)", core::AngleMode::Radians), -1.0); }

    void stepsFollowPrecedence()
    {
        core::Environment environment;
        core::Evaluator evaluator(environment, core::MathLibrary::standard());
        evaluator.evaluate(*core::parse("2 + 3 × 4"));

        const auto& steps = evaluator.steps();
        QCOMPARE(steps.size(), std::size_t{2});
        QCOMPARE(test::q(steps[0].expression), QStringLiteral("3 × 4"));
        QCOMPARE(test::q(steps[0].result), QStringLiteral("12"));
        QCOMPARE(test::q(steps[1].expression), QStringLiteral("2 + 12"));
        QCOMPARE(test::q(steps[1].result), QStringLiteral("14"));
    }

    void assignmentStoresVariable()
    {
        core::Environment environment;
        core::Evaluator evaluator(environment, core::MathLibrary::standard());
        evaluator.evaluate(*core::parse("r = 3"));
        QCOMPARE(environment.variable("r"), std::optional<double>(3.0));
        QCOMPARE(evaluator.evaluate(*core::parse("2r^2")), 18.0);
    }

    void errors_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("kind");

        QTest::newRow("divide by zero") << "1 / 0" << int(core::ErrorKind::Math);
        QTest::newRow("sqrt negative") << "sqrt(-1)" << int(core::ErrorKind::Math);
        QTest::newRow("log zero") << "ln(0)" << int(core::ErrorKind::Math);
        QTest::newRow("tan 90") << "tan(90)" << int(core::ErrorKind::Math);
        QTest::newRow("fractional factorial") << "2.5!" << int(core::ErrorKind::Math);
        QTest::newRow("negative fractional power") << "(-8)^(1/3)" << int(core::ErrorKind::Math);
        QTest::newRow("overflow") << "10^400" << int(core::ErrorKind::Math);
        QTest::newRow("unknown variable") << "y + 1" << int(core::ErrorKind::Name);
        QTest::newRow("unknown function") << "foo(1)" << int(core::ErrorKind::Name);
        QTest::newRow("function without call") << "sin + 1" << int(core::ErrorKind::Name);
        QTest::newRow("assign to constant") << "pi = 3" << int(core::ErrorKind::Name);
        QTest::newRow("wrong arity") << "max(1)" << int(core::ErrorKind::Syntax);
    }

    void errors()
    {
        QFETCH(QString, input);
        QFETCH(int, kind);
        const auto error = test::captureError([&] { test::evaluate(input.toStdString()); });
        QVERIFY2(error.has_value(), qPrintable(input));
        QCOMPARE(int(error->kind()), kind);
        QVERIFY(error->position().has_value());
    }

    void formatting()
    {
        QCOMPARE(test::q(core::formatNumber(0.1 + 0.2)), QStringLiteral("0.3"));
        QCOMPARE(test::q(core::formatNumber(-0.0)), QStringLiteral("0"));
        QCOMPARE(test::q(core::formatNumber(-2.5)), QStringLiteral("−2.5"));
        QCOMPARE(test::q(core::formatNumber(1.0 / 3.0)), QStringLiteral("0.333333333333"));
    }
};

QTEST_APPLESS_MAIN(EvaluatorTest)
#include "tst_evaluator.moc"
