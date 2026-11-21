#include "ui/widgets/AstDiagramWidget.h"

#include "ui/Strings.h"
#include "ui/Theme.h"

#include <QPainter>
#include <QPainterPath>

#include <algorithm>

namespace calc::ui {
namespace {

constexpr double kNodeHeight = 38.0;
constexpr double kMinNodeWidth = 46.0;
constexpr double kSiblingGap = 18.0;
constexpr double kLevelGap = 70.0;
constexpr double kMargin = 28.0;

}  // namespace

AstDiagramWidget::AstDiagramWidget(QWidget* parent)
    : QWidget(parent)
{
    QFont font = theme::monospaceFont(12);
    font.setBold(true);
    setFont(font);
}

void AstDiagramWidget::setTree(const std::optional<core::AstOutline>& tree)
{
    boxes_.clear();
    contentSize_ = {};
    if (tree) {
        place(*tree, 0.0, 0);
        for (const Box& box : boxes_) {
            contentSize_.setWidth(std::max(contentSize_.width(), box.rect.right()));
            contentSize_.setHeight(std::max(contentSize_.height(), box.rect.bottom()));
        }
    }
    setMinimumSize((contentSize_ + QSizeF(2 * kMargin, 2 * kMargin)).toSize());
    update();
}

double AstDiagramWidget::nodeWidth(const core::AstOutline& node) const
{
    return std::max(kMinNodeWidth, fontMetrics().horizontalAdvance(toQString(node.label)) + 28.0);
}

double AstDiagramWidget::subtreeWidth(const core::AstOutline& node) const
{
    double children = 0.0;
    for (const core::AstOutline& child : node.children) children += subtreeWidth(child);
    if (!node.children.empty()) children += kSiblingGap * static_cast<double>(node.children.size() - 1);
    return std::max(nodeWidth(node), children);
}

/// Classic tidy layout: children side by side, each parent centred above them.
int AstDiagramWidget::place(const core::AstOutline& node, double left, int depth)
{
    const int index = static_cast<int>(boxes_.size());
    boxes_.push_back(Box{toQString(node.label), node.kind, node.stepNumber, {}, {}});

    const double width = subtreeWidth(node);
    double childrenWidth = 0.0;
    for (const core::AstOutline& child : node.children) childrenWidth += subtreeWidth(child);
    if (!node.children.empty()) childrenWidth += kSiblingGap * static_cast<double>(node.children.size() - 1);

    double x = left + (width - childrenWidth) / 2.0;
    std::vector<int> children;
    for (const core::AstOutline& child : node.children) {
        children.push_back(place(child, x, depth + 1));
        x += subtreeWidth(child) + kSiblingGap;
    }

    const double center = children.empty()
                              ? left + width / 2.0
                              : (boxes_[static_cast<std::size_t>(children.front())].rect.center().x() +
                                 boxes_[static_cast<std::size_t>(children.back())].rect.center().x()) / 2.0;
    const double w = nodeWidth(node);
    Box& box = boxes_[static_cast<std::size_t>(index)];
    box.rect = QRectF(center - w / 2.0, depth * kLevelGap, w, kNodeHeight);
    box.children = std::move(children);
    return index;
}

void AstDiagramWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (boxes_.empty()) {
        painter.setPen(theme::kTextMuted);
        painter.drawText(rect(), Qt::AlignCenter, tr("Type an expression to see its syntax tree"));
        return;
    }

    const QPointF offset(std::max(kMargin, (width() - contentSize_.width()) / 2.0), kMargin);

    // Edges first, so nodes sit on top of them.
    QColor edgeColor = theme::kTextMuted;
    edgeColor.setAlpha(140);
    painter.setPen(QPen(edgeColor, 2));
    painter.setBrush(Qt::NoBrush);
    for (const Box& box : boxes_) {
        const QPointF from = box.rect.translated(offset).center() + QPointF(0, kNodeHeight / 2.0);
        for (const int childIndex : box.children) {
            const QPointF to = boxes_[static_cast<std::size_t>(childIndex)].rect.translated(offset).center() -
                               QPointF(0, kNodeHeight / 2.0);
            QPainterPath path(from);
            const double bend = (to.y() - from.y()) / 2.0;
            path.cubicTo(from + QPointF(0, bend), to - QPointF(0, bend), to);
            painter.drawPath(path);
        }
    }

    QFont badgeFont = font();
    badgeFont.setPointSizeF(font().pointSizeF() * 0.72);

    for (const Box& box : boxes_) {
        const QRectF rect = box.rect.translated(offset);
        const QColor color = theme::colorFor(box.kind);
        QColor fill = color;
        fill.setAlpha(38);

        painter.setPen(QPen(color, 2));
        painter.setBrush(theme::kBackground);
        painter.drawRoundedRect(rect, kNodeHeight / 2.0, kNodeHeight / 2.0);
        painter.setBrush(fill);
        painter.drawRoundedRect(rect, kNodeHeight / 2.0, kNodeHeight / 2.0);

        painter.setFont(font());
        painter.setPen(color);
        painter.drawText(rect, Qt::AlignCenter, box.label);

        if (box.stepNumber && box.kind != core::OutlineKind::Number) {
            const QRectF badge(rect.right() - 10, rect.top() - 10, 22, 22);
            painter.setPen(QPen(theme::kBackground, 2));
            painter.setBrush(theme::kAccent);
            painter.drawEllipse(badge);
            painter.setFont(badgeFont);
            painter.setPen(Qt::white);
            painter.drawText(badge, Qt::AlignCenter, QString::number(*box.stepNumber));
        }
    }
}

}  // namespace calc::ui
