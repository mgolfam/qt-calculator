#include "TestHelpers.h"

#include "core/AstPrinter.h"

#include <QtTest>

using namespace calc;

class ParserTest : public QObject {
    Q_OBJECT

private slots:
    void precedence_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("infix");
        QTest::addColumn<QString>("postfix");

        QTest::newRow("× before +") << "2+3*4" << "2 + (3 × 4)" << "2 3 4 × +";
        QTest::newRow("brackets") << "(2+3)*4" << "(2 + 3) × 4" << "2 3 + 4 ×";
        QTest::newRow("left assoc −") << "10-3-2" << "(10 − 3) − 2" << "10 3 − 2 −";
        QTest::newRow("right assoc ^") << "2^3^2" << "2 ^ (3 ^ 2)" << "2 3 2 ^ ^";
        QTest::newRow("^ before negate") << "-2^2" << "−(2 ^ 2)" << "2 2 ^ neg";
        QTest::newRow("negative exponent") << "2^-1" << "2 ^ (−1)" << "2 1 neg ^";
        QTest::newRow("factorial") << "3!^2" << "3! ^ 2" << "3 ! 2 ^";
        QTest::newRow("implicit ×") << "2x" << "2 × x" << "2 x ×";
        QTest::newRow("implicit with brackets") << "3(4+1)" << "3 × (4 + 1)" << "3 4 1 + ×";
        QTest::newRow("implicit binds power") << "2x^2" << "2 × (x ^ 2)" << "2 x 2 ^ ×";
        QTest::newRow("call") << "max(1+2, 3)" << "max(1 + 2, 3)" << "1 2 + 3 max";
        QTest::newRow("assignment") << "r = 1 + 2" << "r = 1 + 2" << "1 2 + →r";
        QTest::newRow("pi") << "2π" << "2 × π" << "2 π ×";
    }

    void precedence()
    {
        QFETCH(QString, input);
        QFETCH(QString, infix);
        QFETCH(QString, postfix);
        const core::NodePtr ast = core::parse(input.toStdString());
        QCOMPARE(test::q(core::toInfix(*ast)), infix);
        QCOMPARE(test::q(core::toPostfix(*ast)), postfix);
    }

    void syntaxErrors_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("position");

        QTest::newRow("empty") << "   " << 3;
        QTest::newRow("dangling operator") << "1 +" << 3;
        QTest::newRow("missing close") << "(1 + 2" << 6;
        QTest::newRow("extra close") << "1 + 2)" << 5;
        QTest::newRow("two numbers") << "2 3" << 2;
        QTest::newRow("empty brackets") << "()" << 1;
        QTest::newRow("assignment without value") << "x =" << 3;
    }

    void syntaxErrors()
    {
        QFETCH(QString, input);
        QFETCH(int, position);
        const auto error = test::captureError([&] { core::parse(input.toStdString()); });
        QVERIFY(error.has_value());
        QCOMPARE(error->kind(), core::ErrorKind::Syntax);
        QCOMPARE(error->position(), std::optional<std::size_t>(static_cast<std::size_t>(position)));
    }

    void outlineNumbersMatchEvaluationOrder()
    {
        // 2 + 3 × 4 : step 1 is ×, step 2 is +
        const core::AstOutline tree = core::outline(*core::parse("2 + 3 * 4"));
        QCOMPARE(tree.stepNumber, std::optional<int>(2));
        QCOMPARE(tree.children.size(), std::size_t{2});
        QCOMPARE(tree.children[0].stepNumber, std::optional<int>());
        QCOMPARE(tree.children[1].stepNumber, std::optional<int>(1));
    }
};

QTEST_APPLESS_MAIN(ParserTest)
#include "tst_parser.moc"
