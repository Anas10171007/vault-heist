#pragma once

#include <string>

#include "game/tree/TreeLayout.h"
#include "game/tree/TreeMotion.h"
#include "gui/core/AnimationController.h"
#include "gui/core/Renderer.h"
#include "model/EventLog.h"
#include "model/tree/BinaryTree.h"

namespace vh {

// Replays model events as absolute layout frames. It never keeps a node
// position of its own: every draw asks TreeMotion for the frame at the current
// replay index, so skip-to-end and undo always land on a recorded layout.
class TreeAnimationView : public AnimationListener {
public:
    TreeAnimationView();

    void setModel(const BinaryTree& tree) { model = &tree; }
    void setMotion(const TreeMotion& script) { motion = script; }
    void setOrigin(float x, float y) { originX = x; originY = y; }

    void reset() override;
    void onEvent(const Event& event, float progress) override;
    void draw(Renderer& renderer) const;

private:
    struct ScreenPoint {
        float x = 0.f;
        float y = 0.f;
        bool valid = false;
    };

    ScreenPoint toScreen(const LayoutPoint& point, const TreeLayout& layout) const;
    ScreenPoint pointFor(int nodeId, const TreeLayout& layout) const;
    int nodeStateFor(int nodeId, const TreeLayout& layout) const;
    bool isInHighlightedSubtree(int nodeId, int rootId) const;
    std::string labelFor(int nodeId) const;
    void drawEdges(Renderer& renderer, const TreeLayout& layout) const;
    void drawEventDecorations(Renderer& renderer, const TreeLayout& layout) const;

    const BinaryTree* model = nullptr;
    TreeMotion motion;
    float originX = 40.f;
    float originY = 60.f;
    int replayCount = 0;
    int activeIndex = -1;
    Event activeEvent;
    float activeProgress = 0.f;
};

}
