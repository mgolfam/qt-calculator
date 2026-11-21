#include "infrastructure/JsonHistoryRepository.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using calc::application::HistoryEntry;
using calc::infrastructure::JsonHistoryRepository;

class JsonHistoryRepositoryTest : public QObject {
    Q_OBJECT

private slots:
    void roundTrip()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("nested/history.json"));

        JsonHistoryRepository writer(path);
        writer.save({{"2 + 3 × 4", "14"}, {"r = 3", "3"}});

        JsonHistoryRepository reader(path);
        const std::vector<HistoryEntry> entries = reader.load();
        QCOMPARE(entries.size(), std::size_t{2});
        QCOMPARE(QString::fromStdString(entries[0].expression), QStringLiteral("2 + 3 × 4"));
        QCOMPARE(QString::fromStdString(entries[1].result), QStringLiteral("3"));
    }

    void missingFileIsEmpty()
    {
        QTemporaryDir directory;
        QVERIFY(JsonHistoryRepository(directory.filePath(QStringLiteral("none.json"))).load().empty());
    }

    void corruptFileIsIgnored()
    {
        QTemporaryDir directory;
        const QString path = directory.filePath(QStringLiteral("history.json"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ not json");
        file.close();

        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Ignoring unreadable history file.*")));
        QVERIFY(JsonHistoryRepository(path).load().empty());
    }
};

QTEST_GUILESS_MAIN(JsonHistoryRepositoryTest)
#include "tst_json_history_repository.moc"
