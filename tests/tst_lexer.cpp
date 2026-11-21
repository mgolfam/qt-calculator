#include "TestHelpers.h"

#include "core/Lexer.h"

#include <QtTest>

using namespace calc;
using core::TokenType;

class LexerTest : public QObject {
    Q_OBJECT

private slots:
    void numbers_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<double>("value");
        QTest::newRow("integer") << "42" << 42.0;
        QTest::newRow("decimal") << "3.25" << 3.25;
        QTest::newRow("leading dot") << ".5" << 0.5;
        QTest::newRow("scientific") << "6.02e23" << 6.02e23;
        QTest::newRow("negative exponent") << "1e-3" << 0.001;
    }

    void numbers()
    {
        QFETCH(QString, input);
        QFETCH(double, value);
        const auto tokens = core::Lexer::tokenize(input.toStdString());
        QCOMPARE(tokens.size(), std::size_t{2});
        QCOMPARE(tokens[0].type, TokenType::Number);
        QCOMPARE(tokens[0].value, value);
        QCOMPARE(tokens[1].type, TokenType::End);
    }

    void eAfterNumberIsTheConstant()
    {
        const auto tokens = core::Lexer::tokenize("2e");
        QCOMPARE(tokens[0].type, TokenType::Number);
        QCOMPARE(tokens[1].type, TokenType::Identifier);
        QCOMPARE(test::q(tokens[1].text), QStringLiteral("e"));
    }

    void operatorsAndPositions()
    {
        const auto tokens = core::Lexer::tokenize("sin(x) + 2^3!");
        const std::vector<TokenType> expected{TokenType::Identifier, TokenType::LeftParen, TokenType::Identifier,
                                              TokenType::RightParen, TokenType::Plus,      TokenType::Number,
                                              TokenType::Caret,      TokenType::Number,    TokenType::Bang,
                                              TokenType::End};
        QCOMPARE(tokens.size(), expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) QCOMPARE(tokens[i].type, expected[i]);
        QCOMPARE(tokens[4].position, std::size_t{7});
    }

    void unicodeAliases()
    {
        const auto tokens = core::Lexer::tokenize("2 × π ÷ 3 − 1");
        QCOMPARE(tokens[1].type, TokenType::Star);
        QCOMPARE(tokens[2].type, TokenType::Identifier);
        QCOMPARE(test::q(tokens[2].text), QStringLiteral("pi"));
        QCOMPARE(tokens[3].type, TokenType::Slash);
        QCOMPARE(tokens[5].type, TokenType::Minus);
        // Positions count characters, not UTF-8 bytes.
        QCOMPARE(tokens[3].position, std::size_t{6});
    }

    void doubleStarIsPower()
    {
        const auto tokens = core::Lexer::tokenize("2**3");
        QCOMPARE(tokens[1].type, TokenType::Caret);
    }

    void rejectsUnknownCharacters()
    {
        const auto error = test::captureError([] { core::Lexer::tokenize("2 # 3"); });
        QVERIFY(error.has_value());
        QCOMPARE(error->kind(), core::ErrorKind::Syntax);
        QCOMPARE(error->position(), std::optional<std::size_t>(2));
    }

    void rejectsTwoDecimalPoints()
    {
        QVERIFY(test::captureError([] { core::Lexer::tokenize("1.2.3"); }).has_value());
    }
};

QTEST_APPLESS_MAIN(LexerTest)
#include "tst_lexer.moc"
