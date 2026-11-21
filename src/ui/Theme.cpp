#include "ui/Theme.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QWidget>

namespace calc::ui::theme {

QColor colorFor(core::OutlineKind kind)
{
    switch (kind) {
    case core::OutlineKind::Number: return {0x38, 0xBD, 0xF8};
    case core::OutlineKind::Variable: return {0x4A, 0xDE, 0x80};
    case core::OutlineKind::Operator: return {0xFB, 0xBF, 0x24};
    case core::OutlineKind::Function: return {0xC0, 0x84, 0xFC};
    case core::OutlineKind::Assignment: return {0xF4, 0x72, 0xB6};
    }
    return kText;
}

QColor curveColor(int index)
{
    static const QColor kCurves[] = {
        kAccentAlt, {0xF4, 0x72, 0xB6}, kWarning, {0x4A, 0xDE, 0x80}, {0xC0, 0x84, 0xFC},
    };
    constexpr int count = static_cast<int>(sizeof(kCurves) / sizeof(kCurves[0]));
    return kCurves[((index % count) + count) % count];
}

QFont monospaceFont(int pointSize)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(pointSize);
    return font;
}

void apply(QApplication& app)
{
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette palette;
    palette.setColor(QPalette::Window, kBackground);
    palette.setColor(QPalette::WindowText, kText);
    palette.setColor(QPalette::Base, kBackground);
    palette.setColor(QPalette::AlternateBase, kSurface);
    palette.setColor(QPalette::ToolTipBase, kSurfaceRaised);
    palette.setColor(QPalette::ToolTipText, kText);
    palette.setColor(QPalette::Text, kText);
    palette.setColor(QPalette::PlaceholderText, kTextMuted);
    palette.setColor(QPalette::Button, kSurfaceRaised);
    palette.setColor(QPalette::ButtonText, kText);
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::Highlight, kAccent);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::Link, kAccentAlt);
    palette.setColor(QPalette::Disabled, QPalette::Text, kTextMuted);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, kTextMuted);
    QApplication::setPalette(palette);

    QFile styleSheet(QStringLiteral(":/styles/dark.qss"));
    if (styleSheet.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleSheet.readAll()));
    }
}

void setStyleState(QWidget* widget, const char* property, const QVariant& value)
{
    if (widget->property(property) == value) return;
    widget->setProperty(property, value);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

}  // namespace calc::ui::theme
