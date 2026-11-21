#pragma once

#include "application/HistoryRepository.h"

#include <QString>

namespace calc::infrastructure {

/// Adapter: persists history as a small JSON file in the user's app-data folder.
class JsonHistoryRepository final : public application::HistoryRepository {
public:
    explicit JsonHistoryRepository(QString filePath);

    /// e.g. ~/.local/share/<Organization>/<Application>/history.json on Linux.
    static QString defaultFilePath();

    std::vector<application::HistoryEntry> load() override;
    void save(const std::vector<application::HistoryEntry>& entries) override;

private:
    QString filePath_;
};

}  // namespace calc::infrastructure
