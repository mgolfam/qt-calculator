#pragma once

#include "core/AstPrinter.h"

#include <QColor>
#include <QFont>
#include <QVariant>

class QApplication;
class QWidget;

/// One place for every colour and font, shared by the stylesheet and custom painting.
namespace calc::ui::theme {

inline const QColor kBackground{0x0E, 0x11, 0x17};
inline const QColor kSurface{0x16, 0x1B, 0x26};
inline const QColor kSurfaceRaised{0x1E, 0x24, 0x33};
inline const QColor kBorder{0x2A, 0x31, 0x42};
inline const QColor kText{0xE6, 0xE9, 0xF2};
inline const QColor kTextMuted{0x8A, 0x93, 0xA8};
inline const QColor kAccent{0x7C, 0x5C, 0xFF};
inline const QColor kAccentAlt{0x22, 0xD3, 0xEE};
inline const QColor kSuccess{0x34, 0xD3, 0x99};
inline const QColor kWarning{0xFB, 0xBF, 0x24};
inline const QColor kError{0xF8, 0x71, 0x71};

QColor colorFor(core::OutlineKind kind);
QColor curveColor(int index);
QFont monospaceFont(int pointSize);

/// Fusion style + dark palette + the bundled stylesheet.
void apply(QApplication& app);

/// Sets a dynamic property used by the stylesheet (e.g. state="error") and re-polishes.
void setStyleState(QWidget* widget, const char* property, const QVariant& value);

}  // namespace calc::ui::theme
