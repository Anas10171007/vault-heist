#pragma once

#include "model/Node.h"
#include "util/DynArray.h"

namespace vh {

class BinaryTree;

struct LayoutPoint {
    int nodeId = 0;
    float x = 0.f;
    float y = 0.f;

    LayoutPoint() = default;
    LayoutPoint(int id, float px, float py) : nodeId(id), x(px), y(py) {}
};

struct LayoutBox {
    float minX = 0.f;
    float minY = 0.f;
    float maxX = 0.f;
    float maxY = 0.f;

    float width() const { return maxX - minX; }
    float height() const { return maxY - minY; }
};

// Pure geometry. No drawing and no state is kept between calls, so a caller can
// compute a layout for the current tree before a rotation is animated.
class TreeLayout {
public:
    TreeLayout() = default;

    static TreeLayout compute(const BinaryTree& tree, float xSpacing = 64.f,
                              float ySpacing = 90.f);
    static TreeLayout compute(const TreeNode* root, float xSpacing = 64.f,
                              float ySpacing = 90.f);

    bool empty() const { return points.empty(); }
    int size() const { return points.size(); }
    bool contains(int nodeId) const;
    const LayoutPoint& at(int index) const { return points.at(index); }
    const LayoutBox& bounds() const { return box; }

    // Keyed lookup. Missing ids return 0.
    float xOf(int nodeId) const;
    float yOf(int nodeId) const;

private:
    DynArray<LayoutPoint> points;
    LayoutBox box;
};

}
