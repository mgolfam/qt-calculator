#pragma once

#include "core/AstPrinter.h"

#include <QRectF>
#include <QWidget>

#include <optional>
#include <vector>

namespace calc::ui {

/// Draws the syntax tree. Each operation is badged with its step number, so the
/// tree and the Steps tab tell the same story.
class AstDiagramWidget : public QWidget {
    Q_OBJECT

public:
    explicit AstDiagramWidget(QWidget* parent = nullptr);

    void setTree(const std::optional<core::AstOutline>& tree);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct Box {
        QString label;
        core::OutlineKind kind;
        std::optional<int> stepNumber;
        QRectF rect;
        std::vector<int> children;
    };

    double nodeWidth(const core::AstOutline& node) const;
    double subtreeWidth(const core::AstOutline& node) const;
    int place(const core::AstOutline& node, double left, int depth);

    std::vector<Box> boxes_;
    QSizeF contentSize_;
};

}  // namespace calc::ui
