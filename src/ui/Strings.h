#pragma once

#include <QString>

#include <string>

namespace calc::ui {

/// The core speaks UTF-8 std::string; Qt speaks QString. Convert only at the edge.
inline QString toQString(const std::string& text) { return QString::fromStdString(text); }
inline std::string toStdString(const QString& text) { return text.toStdString(); }

}  // namespace calc::ui
