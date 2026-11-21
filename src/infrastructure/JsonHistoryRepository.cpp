#include "infrastructure/JsonHistoryRepository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QtDebug>

namespace calc::infrastructure {
namespace {

constexpr int kFormatVersion = 1;

}  // namespace

JsonHistoryRepository::JsonHistoryRepository(QString filePath)
    : filePath_(std::move(filePath))
{
}

QString JsonHistoryRepository::defaultFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/history.json");
}

std::vector<application::HistoryEntry> JsonHistoryRepository::load()
{
    QFile file(filePath_);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return {};

    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        qWarning() << "Ignoring unreadable history file" << filePath_ << parseError.errorString();
        return {};
    }

    std::vector<application::HistoryEntry> entries;
    const QJsonArray array = document.object().value(QStringLiteral("entries")).toArray();
    entries.reserve(static_cast<std::size_t>(array.size()));
    for (const QJsonValue& value : array) {
        const QJsonObject object = value.toObject();
        const QString expression = object.value(QStringLiteral("expression")).toString();
        if (expression.isEmpty()) continue;
        entries.push_back({expression.toStdString(), object.value(QStringLiteral("result")).toString().toStdString()});
    }
    return entries;
}

void JsonHistoryRepository::save(const std::vector<application::HistoryEntry>& entries)
{
    QJsonArray array;
    for (const application::HistoryEntry& entry : entries) {
        array.append(QJsonObject{
            {QStringLiteral("expression"), QString::fromStdString(entry.expression)},
            {QStringLiteral("result"), QString::fromStdString(entry.result)},
        });
    }
    const QJsonObject root{{QStringLiteral("version"), kFormatVersion}, {QStringLiteral("entries"), array}};

    QDir().mkpath(QFileInfo(filePath_).absolutePath());
    QSaveFile file(filePath_);  // atomic: never leaves a half-written file behind
    if (!file.open(QIODevice::WriteOnly) || file.write(QJsonDocument(root).toJson()) < 0 || !file.commit()) {
        qWarning() << "Could not save history to" << filePath_ << file.errorString();
    }
}

}  // namespace calc::infrastructure
