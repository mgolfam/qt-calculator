#include "ui/widgets/GraphWidget.h"

#include "ui/Theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace calc::ui {
namespace {

constexpr double kDefaultScale = 40.0;    // pixels per unit
constexpr double kTargetGridPixels = 70.0;
constexpr double kMinScale = 1e-4;
constexpr double kMaxScale = 1e7;

QString formatTick(double value, double step)
{
    if (std::abs(value) < step * 1e-9) return QStringLiteral("0");
    return QString::number(value, 'g', 6);
}

}  // namespace

GraphWidget::GraphWidget(QWidget* parent)
    : QWidget(parent)
    , scale_(kDefaultScale)
{
    setMouseTracking(true);
    setMinimumSize(320, 240);
    setCursor(Qt::CrossCursor);
}

void GraphWidget::setCurves(std::vector<Curve> curves)
{
    curves_ = std::move(curves);
    update();
}

void GraphWidget::resetView()
{
    center_ = {0.0, 0.0};
    scale_ = kDefaultScale;
    update();
}

QPointF GraphWidget::toScreen(QPointF world) const
{
    return {width() / 2.0 + (world.x() - center_.x()) * scale_, height() / 2.0 - (world.y() - center_.y()) * scale_};
}

QPointF GraphWidget::toWorld(QPointF screen) const
{
    return {center_.x() + (screen.x() - width() / 2.0) / scale_, center_.y() - (screen.y() - height() / 2.0) / scale_};
}

/// A "nice" grid spacing (1, 2 or 5 × 10ⁿ) close to kTargetGridPixels on screen.
double GraphWidget::gridStep() const
{
    const double raw = kTargetGridPixels / scale_;
    const double magnitude = std::pow(10.0, std::floor(std::log10(raw)));
    const double normalized = raw / magnitude;
    const double nice = normalized <= 1.0 ? 1.0 : normalized <= 2.0 ? 2.0 : normalized <= 5.0 ? 5.0 : 10.0;
    return nice * magnitude;
}

void GraphWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath frame;
    frame.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);
    painter.setClipPath(frame);
    painter.fillRect(rect(), theme::kBackground);

    drawGrid(painter);
    drawAxes(painter);
    drawCurves(painter);
    drawHover(painter);
    drawLegend(painter);

    painter.setClipping(false);
    painter.setPen(QPen(theme::kBorder, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(frame);
}

void GraphWidget::drawGrid(QPainter& painter) const
{
    const double step = gridStep();
    const QPointF topLeft = toWorld({0, 0});
    const QPointF bottomRight = toWorld({static_cast<double>(width()), static_cast<double>(height())});

    auto drawLines = [&](double spacing, const QColor& color) {
        painter.setPen(QPen(color, 1));
        for (auto i = static_cast<long long>(std::floor(topLeft.x() / spacing)); i * spacing <= bottomRight.x(); ++i) {
            const double x = toScreen({i * spacing, 0}).x();
            painter.drawLine(QPointF(x, 0), QPointF(x, height()));
        }
        for (auto i = static_cast<long long>(std::floor(bottomRight.y() / spacing)); i * spacing <= topLeft.y(); ++i) {
            const double y = toScreen({0, i * spacing}).y();
            painter.drawLine(QPointF(0, y), QPointF(width(), y));
        }
    };

    QColor minor = theme::kSurfaceRaised;
    minor.setAlpha(110);
    drawLines(step / 5.0, minor);
    drawLines(step, theme::kSurfaceRaised);
}

void GraphWidget::drawAxes(QPainter& painter) const
{
    const QPointF origin = toScreen({0, 0});
    painter.setPen(QPen(theme::kTextMuted, 1.4));
    if (origin.x() >= 0 && origin.x() <= width()) painter.drawLine(QPointF(origin.x(), 0), QPointF(origin.x(), height()));
    if (origin.y() >= 0 && origin.y() <= height()) painter.drawLine(QPointF(0, origin.y()), QPointF(width(), origin.y()));

    // Tick labels stick to the edge when the axis scrolls out of view.
    const double step = gridStep();
    const QPointF topLeft = toWorld({0, 0});
    const QPointF bottomRight = toWorld({static_cast<double>(width()), static_cast<double>(height())});
    const double labelY = std::clamp(origin.y() + 16.0, 16.0, height() - 6.0);
    const double labelX = std::clamp(origin.x() + 6.0, 6.0, width() - 60.0);

    QFont font = painter.font();
    font.setPointSizeF(8.5);
    painter.setFont(font);
    painter.setPen(theme::kTextMuted);

    for (auto i = static_cast<long long>(std::floor(topLeft.x() / step)); i * step <= bottomRight.x(); ++i) {
        if (i == 0) continue;
        const double x = toScreen({i * step, 0}).x();
        painter.drawText(QPointF(x + 3, labelY), formatTick(i * step, step));
    }
    for (auto i = static_cast<long long>(std::floor(bottomRight.y() / step)); i * step <= topLeft.y(); ++i) {
        if (i == 0) continue;
        const double y = toScreen({0, i * step}).y();
        painter.drawText(QPointF(labelX, y - 3), formatTick(i * step, step));
    }
    painter.drawText(QPointF(labelX, labelY), QStringLiteral("0"));
}

void GraphWidget::drawCurves(QPainter& painter) const
{
    const double h = height();
    for (const Curve& curve : curves_) {
        painter.setPen(QPen(curve.color, 2.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        QPolygonF segment;
        auto flush = [&] {
            if (segment.size() > 1) painter.drawPolyline(segment);
            segment.clear();
        };

        for (int px = 0; px <= width(); ++px) {
            const double x = toWorld({static_cast<double>(px), 0}).x();
            const std::optional<double> y = curve.function(x);
            if (!y) {
                flush();
                continue;
            }
            const double sy = toScreen({x, *y}).y();
            // A huge vertical jump between neighbouring pixels is an asymptote (e.g. tan) — don't connect it.
            if (!segment.isEmpty() && std::abs(sy - segment.last().y()) > h) flush();
            segment << QPointF(px, std::clamp(sy, -2.0 * h, 3.0 * h));
        }
        flush();
    }
}

void GraphWidget::drawHover(QPainter& painter) const
{
    if (!hover_ || dragStart_ || curves_.empty()) return;

    const double x = toWorld(*hover_).x();
    painter.setPen(QPen(theme::kTextMuted, 1, Qt::DashLine));
    painter.drawLine(QPointF(hover_->x(), 0), QPointF(hover_->x(), height()));

    QStringList lines{QStringLiteral("x = %1").arg(x, 0, 'g', 6)};
    for (const Curve& curve : curves_) {
        const std::optional<double> y = curve.function(x);
        if (!y) continue;
        const QPointF point = toScreen({x, *y});
        painter.setPen(QPen(theme::kBackground, 2));
        painter.setBrush(curve.color);
        painter.drawEllipse(point, 5, 5);
        lines << QStringLiteral("%1 = %2").arg(curve.label).arg(*y, 0, 'g', 6);
    }

    const QFontMetrics metrics(painter.font());
    int boxWidth = 0;
    for (const QString& line : lines) boxWidth = std::max(boxWidth, metrics.horizontalAdvance(line));
    const QSizeF size(boxWidth + 20, lines.size() * metrics.height() + 12);
    QPointF topLeft = *hover_ + QPointF(14, 14);
    if (topLeft.x() + size.width() > width()) topLeft.setX(hover_->x() - size.width() - 14);
    if (topLeft.y() + size.height() > height()) topLeft.setY(hover_->y() - size.height() - 14);

    const QRectF box(topLeft, size);
    painter.setPen(QPen(theme::kBorder, 1));
    painter.setBrush(theme::kSurfaceRaised);
    painter.drawRoundedRect(box, 8, 8);
    painter.setPen(theme::kText);
    for (int i = 0; i < lines.size(); ++i) {
        painter.drawText(QPointF(box.left() + 10, box.top() + 6 + metrics.ascent() + i * metrics.height()), lines[i]);
    }
}

void GraphWidget::drawLegend(QPainter& painter) const
{
    if (curves_.empty()) return;
    const QFontMetrics metrics(painter.font());
    double y = 14;
    for (const Curve& curve : curves_) {
        const QString text = QStringLiteral("y = ") + curve.label;
        const QRectF pill(12, y, metrics.horizontalAdvance(text) + 34, metrics.height() + 8);
        painter.setPen(Qt::NoPen);
        QColor background = theme::kSurfaceRaised;
        background.setAlpha(225);
        painter.setBrush(background);
        painter.drawRoundedRect(pill, 8, 8);
        painter.setBrush(curve.color);
        painter.drawEllipse(QPointF(pill.left() + 13, pill.center().y()), 5, 5);
        painter.setPen(theme::kText);
        painter.drawText(QPointF(pill.left() + 24, pill.top() + 4 + metrics.ascent()), text);
        y += pill.height() + 6;
    }
}

void GraphWidget::wheelEvent(QWheelEvent* event)
{
    const QPointF anchorScreen = event->position();
    const QPointF anchorWorld = toWorld(anchorScreen);
    const double factor = std::pow(1.0015, event->angleDelta().y());
    scale_ = std::clamp(scale_ * factor, kMinScale, kMaxScale);
    // Keep the point under the cursor fixed while zooming.
    center_ = {anchorWorld.x() - (anchorScreen.x() - width() / 2.0) / scale_,
               anchorWorld.y() + (anchorScreen.y() - height() / 2.0) / scale_};
    update();
    event->accept();
}

void GraphWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;
    dragStart_ = event->position();
    dragStartCenter_ = center_;
    setCursor(Qt::ClosedHandCursor);
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event)
{
    hover_ = event->position();
    if (dragStart_) {
        const QPointF delta = event->position() - *dragStart_;
        center_ = dragStartCenter_ - QPointF(delta.x() / scale_, -delta.y() / scale_);
    }
    update();
}

void GraphWidget::mouseReleaseEvent(QMouseEvent*)
{
    dragStart_.reset();
    setCursor(Qt::CrossCursor);
    update();
}

void GraphWidget::mouseDoubleClickEvent(QMouseEvent*) { resetView(); }

void GraphWidget::leaveEvent(QEvent*)
{
    hover_.reset();
    update();
}

}  // namespace calc::ui
