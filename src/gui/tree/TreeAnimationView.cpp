#include "gui/tree/TreeAnimationView.h"

#include <string>

#include "gui/core/Renderer.h"
#include "gui/core/Style.h"
#include "model/Event.h"
#include "model/Node.h"

namespace vh {

namespace {

const char* rotationName(int type) {
    if (type == RotationType::LL) return "LL";
    if (type == RotationType::RR) return "RR";
    if (type == RotationType::LR) return "LR";
    if (type == RotationType::RL) return "RL";
    return "ROT";
}

int highlightState(int mode) {
    if (mode == HighlightMode::VISITED) return NodeState::VISITED;
    if (mode == HighlightMode::SKIPPED) return NodeState::SKIPPED;
    return NodeState::TARGET;
}

float mix(float from, float to, float amount) {
    return from + (to - from) * amount;
}

}

TreeAnimationView::TreeAnimationView() : activeEvent(EventType::DONE) {}

void TreeAnimationView::reset() {
    replayCount = 0;
    activeIndex = -1;
    activeEvent = Event(EventType::DONE);
    activeProgress = 0.f;
}

void TreeAnimationView::onEvent(const Event& event, float progress) {
    activeIndex = replayCount;
    replayCount++;
    activeEvent = event;
    activeProgress = progress;
}

TreeAnimationView::ScreenPoint TreeAnimationView::toScreen(const LayoutPoint& point,
                                                           const TreeLayout& layout) const {
    (void)layout;
    ScreenPoint result;
    result.x = originX + point.x;
    result.y = originY + point.y;
    result.valid = true;
    return result;
}

TreeAnimationView::ScreenPoint TreeAnimationView::pointFor(int nodeId,
                                                           const TreeLayout& layout) const {
    ScreenPoint result;
    if (!layout.contains(nodeId)) return result;
    LayoutPoint point;
    point.nodeId = nodeId;
    point.x = layout.xOf(nodeId);
    point.y = layout.yOf(nodeId);
    return toScreen(point, layout);
}

std::string TreeAnimationView::labelFor(int nodeId) const {
    if (model == nullptr) return "?";
    TreeNode* node = model->findById(nodeId);
    return node == nullptr ? "?" : std::to_string(node->value);
}

bool TreeAnimationView::isInHighlightedSubtree(int nodeId, int rootId) const {
    if (model == nullptr) return false;
    TreeNode* node = model->findById(nodeId);
    while (node != nullptr) {
        if (node->id == rootId) return true;
        node = node->parent;
    }
    return false;
}

int TreeAnimationView::nodeStateFor(int nodeId, const TreeLayout& layout) const {
    (void)layout;
    if (activeEvent.type == EventType::VISIT && activeEvent.idA == nodeId) return NodeState::VISITED;
    if (activeEvent.type == EventType::COMPARE && activeEvent.idA == nodeId) return NodeState::WARNING;
    if (activeEvent.type == EventType::INSERT && activeEvent.idA == nodeId) return NodeState::TARGET;
    if (activeEvent.type == EventType::UNLINK && activeEvent.idA == nodeId) return NodeState::SKIPPED;
    if (activeEvent.type == EventType::SWAP_VALUE &&
        (activeEvent.idA == nodeId || activeEvent.idB == nodeId)) {
        return NodeState::TARGET;
    }
    if (activeEvent.type == EventType::ROTATE) {
        if (activeEvent.idB == nodeId) return NodeState::TARGET;
        if (activeEvent.idA == nodeId) return NodeState::WARNING;
    }
    if ((activeEvent.type == EventType::UPDATE_HEIGHT ||
         activeEvent.type == EventType::UPDATE_SIZE) &&
        activeEvent.idA == nodeId) {
        return NodeState::VISITED;
    }
    if (activeEvent.type == EventType::BALANCE_ALERT && activeEvent.idA == nodeId) {
        return NodeState::VIOLATION;
    }
    if (activeEvent.type == EventType::HIGHLIGHT_SUBTREE &&
        isInHighlightedSubtree(nodeId, activeEvent.idA)) {
        return highlightState(activeEvent.extra);
    }
    return NodeState::NORMAL;
}

void TreeAnimationView::drawEdges(Renderer& renderer, const TreeLayout& layout) const {
    if (model == nullptr) return;
    for (int i = 0; i < layout.size(); i++) {
        int id = layout.at(i).nodeId;
        TreeNode* node = model->findById(id);
        if (node == nullptr || node->parent == nullptr) continue;
        ScreenPoint child = pointFor(id, layout);
        ScreenPoint parent = pointFor(node->parent->id, layout);
        if (!child.valid || !parent.valid) continue;
        bool active = id == activeEvent.idA || id == activeEvent.idB ||
                      node->parent->id == activeEvent.idA;
        renderer.drawEdge({parent.x, parent.y}, {child.x, child.y}, active);
    }
}

void TreeAnimationView::drawEventDecorations(Renderer& renderer,
                                             const TreeLayout& layout) const {
    if (activeEvent.type == EventType::ROTATE) {
        ScreenPoint rising = pointFor(activeEvent.idB, layout);
        if (rising.valid) {
            renderer.drawBadge({rising.x, rising.y - 42.f}, rotationName(activeEvent.extra),
                               NodeState::TARGET);
        }
    } else if (activeEvent.type == EventType::COMPARE) {
        ScreenPoint point = pointFor(activeEvent.idA, layout);
        if (point.valid) {
            renderer.drawText(activeEvent.extra == 0 ? "CMP >" : "CMP <",
                              {point.x + 28.f, point.y - 22.f}, 14, Palette::WARNING, true);
        }
    } else if (activeEvent.type == EventType::INSERT) {
        ScreenPoint point = pointFor(activeEvent.idA, layout);
        if (point.valid) {
            renderer.drawBadge({point.x, point.y - 42.f}, "NEW", NodeState::TARGET);
        }
    } else if (activeEvent.type == EventType::UNLINK) {
        ScreenPoint point = pointFor(activeEvent.idA, layout);
        if (point.valid) {
            if (model != nullptr) {
                TreeNode* node = model->findById(activeEvent.idA);
                if (node != nullptr && node->parent != nullptr) {
                    ScreenPoint parent = pointFor(node->parent->id, layout);
                    if (parent.valid) {
                        renderer.drawEdge({parent.x, parent.y},
                                          {mix(point.x, parent.x, activeProgress),
                                           mix(point.y, parent.y, activeProgress)},
                                          true);
                    }
                }
            }
            renderer.drawBadge({point.x, point.y - 42.f}, "OUT", NodeState::SKIPPED);
        }
    } else if (activeEvent.type == EventType::SWAP_VALUE) {
        ScreenPoint first = pointFor(activeEvent.idA, layout);
        ScreenPoint second = pointFor(activeEvent.idB, layout);
        if (first.valid && second.valid) {
            renderer.drawText(std::to_string(activeEvent.oldValue),
                              {mix(first.x, second.x, activeProgress),
                               mix(first.y, second.y, activeProgress) - 34.f},
                              15, Palette::TEXT, true);
            renderer.drawText(std::to_string(activeEvent.newValue),
                              {mix(second.x, first.x, activeProgress),
                               mix(second.y, first.y, activeProgress) - 34.f},
                              15, Palette::TEXT, true);
            renderer.drawText("SWAP", {(first.x + second.x) / 2.f, (first.y + second.y) / 2.f - 58.f},
                              13, Palette::MUTED, true);
        }
    } else if (activeEvent.type == EventType::BALANCE_ALERT) {
        ScreenPoint point = pointFor(activeEvent.idA, layout);
        if (point.valid) {
            renderer.drawBadge({point.x, point.y - 42.f},
                               std::string("BAD ") + std::to_string(activeEvent.key),
                               NodeState::VIOLATION);
        }
    } else if (activeEvent.type == EventType::UPDATE_HEIGHT ||
               activeEvent.type == EventType::UPDATE_SIZE) {
        ScreenPoint point = pointFor(activeEvent.idA, layout);
        if (point.valid) {
            renderer.drawBadge({point.x, point.y - 42.f},
                               activeEvent.type == EventType::UPDATE_HEIGHT ? "H" : "S",
                               NodeState::VISITED);
        }
    } else if (activeEvent.type == EventType::HIGHLIGHT_SUBTREE) {
        ScreenPoint point = pointFor(activeEvent.idA, layout);
        if (point.valid) {
            renderer.drawBadge({point.x, point.y - 42.f},
                               NodeState::tag(highlightState(activeEvent.extra)),
                               highlightState(activeEvent.extra));
        }
    }
}

TreeLayout TreeAnimationView::currentLayout() const {
    return motion.eventCount() == 0 ? motion.finalLayout()
                                    : motion.frameAt(activeIndex, activeProgress);
}

bool TreeAnimationView::pickNode(sf::Vector2f worldPoint, int& nodeId) const {
    TreeLayout frame = currentLayout();
    for (int i = 0; i < frame.size(); i++) {
        ScreenPoint point = pointFor(frame.at(i).nodeId, frame);
        float dx = point.x - worldPoint.x;
        float dy = point.y - worldPoint.y;
        if (dx * dx + dy * dy <= Renderer::NODE_HIT_RADIUS * Renderer::NODE_HIT_RADIUS) {
            nodeId = frame.at(i).nodeId;
            return true;
        }
    }
    return false;
}

void TreeAnimationView::draw(Renderer& renderer) const {
    TreeLayout frame = motion.eventCount() == 0 ? motion.finalLayout()
                                                 : motion.frameAt(activeIndex, activeProgress);
    if (showEventLabel) {
        renderer.drawText(eventName(activeEvent.type), {originX, 18.f}, 16, Palette::TEXT, false);
    }
    drawEdges(renderer, frame);
    for (int i = 0; i < frame.size(); i++) {
        int id = frame.at(i).nodeId;
        ScreenPoint point = pointFor(id, frame);
        renderer.drawNode({point.x, point.y}, labelFor(id), nodeStateFor(id, frame));
    }
    drawEventDecorations(renderer, frame);
}

}
