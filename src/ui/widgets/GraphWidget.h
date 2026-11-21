#pragma once

#include <QColor>
#include <QPointF>
#include <QString>
#include <QWidget>

#include <functional>
#include <optional>
#include <vector>

namespace calc::ui {

struct Curve {
    QString label;
    QColor color;
    std::function<std::optional<double>(double)> function;
};

/// A pannable, zoomable plot of y = f(x). Drag to pan, scroll to zoom,
/// double-click to reset, hover to read values.
class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget* parent = nullptr);

    void setCurves(std::vector<Curve> curves);

public slots:
    void resetView();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QPointF toScreen(QPointF world) const;
    QPointF toWorld(QPointF screen) const;
    double gridStep() const;

    void drawGrid(QPainter& painter) const;
    void drawAxes(QPainter& painter) const;
    void drawCurves(QPainter& painter) const;
    void drawHover(QPainter& painter) const;
    void drawLegend(QPainter& painter) const;

    std::vector<Curve> curves_;
    QPointF center_{0.0, 0.0};  // world coordinates at the widget's centre
    double scale_;              // pixels per unit
    std::optional<QPointF> hover_;
    std::optional<QPointF> dragStart_;
    QPointF dragStartCenter_;
};

}  // namespace calc::ui
