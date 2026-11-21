#pragma once

#include <string>
#include <utility>
#include <vector>

namespace calc::application {

struct HistoryEntry {
    std::string expression;
    std::string result;
};

/// Port: where calculation history is kept. The application layer only knows this
/// interface; concrete storage (JSON file, database, …) lives in infrastructure.
class HistoryRepository {
public:
    virtual ~HistoryRepository() = default;
    virtual std::vector<HistoryEntry> load() = 0;
    virtual void save(const std::vector<HistoryEntry>& entries) = 0;
};

/// Adapter that keeps history in memory only — used by tests and as a fallback.
class InMemoryHistoryRepository final : public HistoryRepository {
public:
    InMemoryHistoryRepository() = default;
    explicit InMemoryHistoryRepository(std::vector<HistoryEntry> initial) : entries_(std::move(initial)) {}

    std::vector<HistoryEntry> load() override { return entries_; }
    void save(const std::vector<HistoryEntry>& entries) override { entries_ = entries; }

private:
    std::vector<HistoryEntry> entries_;
};

}  // namespace calc::application
