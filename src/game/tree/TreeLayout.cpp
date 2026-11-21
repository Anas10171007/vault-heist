#include "game/tree/TreeLayout.h"

#include "model/tree/BinaryTree.h"
#include "util/Stack.h"

namespace vh {

namespace {

struct LayoutFrame {
    const TreeNode* node = nullptr;
    int depth = 0;
    int stage = 0;
};

}

TreeLayout TreeLayout::compute(const BinaryTree& tree, float xSpacing, float ySpacing) {
    return compute(tree.root(), xSpacing, ySpacing);
}

TreeLayout TreeLayout::compute(const TreeNode* root, float xSpacing, float ySpacing) {
    TreeLayout layout;
    if (root == nullptr) {
        return layout;
    }

    if (xSpacing <= 0.f) {
        xSpacing = 1.f;
    }
    if (ySpacing <= 0.f) {
        ySpacing = 1.f;
    }

    Stack<LayoutFrame> frames;
    LayoutFrame first;
    first.node = root;
    frames.push(first);
    int column = 0;
    bool hasPoint = false;
    while (!frames.empty()) {
        LayoutFrame frame = frames.top();
        if (frame.stage == 0) {
            frame.stage = 1;
            frames.top() = frame;
            if (frame.node->left != nullptr) {
                LayoutFrame child;
                child.node = frame.node->left;
                child.depth = frame.depth + 1;
                frames.push(child);
            }
        } else if (frame.stage == 1) {
            frame.stage = 2;
            frames.top() = frame;
            float x = static_cast<float>(column) * xSpacing;
            float y = static_cast<float>(frame.depth) * ySpacing;
            layout.points.push(LayoutPoint(frame.node->id, x, y));
            column++;
            if (!hasPoint) {
                layout.box.minX = x;
                layout.box.minY = y;
                layout.box.maxX = x;
                layout.box.maxY = y;
                hasPoint = true;
            } else {
                if (x < layout.box.minX) layout.box.minX = x;
                if (x > layout.box.maxX) layout.box.maxX = x;
                if (y < layout.box.minY) layout.box.minY = y;
                if (y > layout.box.maxY) layout.box.maxY = y;
            }
            if (frame.node->right != nullptr) {
                LayoutFrame child;
                child.node = frame.node->right;
                child.depth = frame.depth + 1;
                frames.push(child);
            }
        } else {
            frames.pop();
        }
    }
    return layout;
}

bool TreeLayout::contains(int nodeId) const {
    for (int i = 0; i < points.size(); i++) {
        if (points[i].nodeId == nodeId) {
            return true;
        }
    }
    return false;
}

float TreeLayout::xOf(int nodeId) const {
    for (int i = 0; i < points.size(); i++) {
        if (points[i].nodeId == nodeId) {
            return points[i].x;
        }
    }
    return 0.f;
}

float TreeLayout::yOf(int nodeId) const {
    for (int i = 0; i < points.size(); i++) {
        if (points[i].nodeId == nodeId) {
            return points[i].y;
        }
    }
    return 0.f;
}

}
