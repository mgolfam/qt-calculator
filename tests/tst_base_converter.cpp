#include "TestHelpers.h"

#include "core/BaseConverter.h"

#include <QtTest>

#include <climits>

using namespace calc;
using core::BaseConverter;

class BaseConverterTest : public QObject {
    Q_OBJECT

private slots:
    void fromDecimal_data()
    {
        QTest::addColumn<qlonglong>("value");
        QTest::addColumn<int>("base");
        QTest::addColumn<QString>("digits");

        QTest::newRow("zero") << 0LL << 2 << "0";
        QTest::newRow("binary") << 156LL << 2 << "10011100";
        QTest::newRow("octal") << 156LL << 8 << "234";
        QTest::newRow("hex") << 255LL << 16 << "FF";
        QTest::newRow("base 36") << 35LL << 36 << "Z";
        QTest::newRow("negative") << -10LL << 2 << "-1010";
        QTest::newRow("min long long") << LLONG_MIN << 16 << "-8000000000000000";
    }

    void fromDecimal()
    {
        QFETCH(qlonglong, value);
        QFETCH(int, base);
        QFETCH(QString, digits);
        QCOMPARE(test::q(BaseConverter::fromDecimal(value, base).digits), digits);
    }

    void repeatedDivisionSteps()
    {
        const auto conversion = BaseConverter::fromDecimal(6, 2);
        QCOMPARE(conversion.steps.size(), std::size_t{4});
        QCOMPARE(test::q(conversion.steps[0]), QStringLiteral("6 ÷ 2 = 3 remainder 0"));
        QCOMPARE(test::q(conversion.steps.back()), QStringLiteral("Read the remainders from bottom to top: 110"));
    }

    void toDecimal_data()
    {
        QTest::addColumn<QString>("digits");
        QTest::addColumn<int>("base");
        QTest::addColumn<qlonglong>("value");

        QTest::newRow("binary") << "1101" << 2 << 13LL;
        QTest::newRow("hex lower") << "ff" << 16 << 255LL;
        QTest::newRow("hex prefix") << "0x1F" << 16 << 31LL;
        QTest::newRow("binary prefix") << "0b101" << 2 << 5LL;
        QTest::newRow("leading zeros") << "000101" << 2 << 5LL;
        QTest::newRow("negative") << "-17" << 8 << -15LL;
        QTest::newRow("max") << "7FFFFFFFFFFFFFFF" << 16 << LLONG_MAX;
    }

    void toDecimal()
    {
        QFETCH(QString, digits);
        QFETCH(int, base);
        QFETCH(qlonglong, value);
        QCOMPARE(BaseConverter::toDecimal(digits.toStdString(), base).value, value);
    }

    void positionalSteps()
    {
        const auto conversion = BaseConverter::toDecimal("101", 2);
        QCOMPARE(test::q(conversion.steps[0]), QStringLiteral("1 × 2² = 1 × 4 = 4"));
        QCOMPARE(test::q(conversion.steps.back()), QStringLiteral("Add the place values: 4 + 0 + 1 = 5"));
    }

    void rejectsInvalidInput()
    {
        QVERIFY(test::captureError([] { BaseConverter::toDecimal("102", 2); }).has_value());
        QVERIFY(test::captureError([] { BaseConverter::toDecimal("G", 16); }).has_value());
        QVERIFY(test::captureError([] { BaseConverter::toDecimal("", 10); }).has_value());
        QVERIFY(test::captureError([] { BaseConverter::toDecimal("8000000000000000", 16); }).has_value());
        QVERIFY(test::captureError([] { BaseConverter::fromDecimal(5, 1); }).has_value());
    }
};

QTEST_APPLESS_MAIN(BaseConverterTest)
#include "tst_base_converter.moc"
