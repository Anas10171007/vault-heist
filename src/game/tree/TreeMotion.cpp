#include "game/tree/TreeMotion.h"

namespace vh {

namespace {

float lerp(float from, float to, float progress) {
    return from + (to - from) * progress;
}

bool movesPositions(int eventType) {
    return eventType == EventType::ROTATE || eventType == EventType::INSERT ||
           eventType == EventType::UNLINK || eventType == EventType::SWAP_VALUE;
}

}

void TreeMotion::reset() {
    baseLayout = TreeLayout();
    finalLayoutData = TreeLayout();
    events.clear();
    layoutsAfter.clear();
}

void TreeMotion::setBase(const TreeLayout& layout) {
    baseLayout = layout;
}

void TreeMotion::addEvent(const Event& event, const TreeLayout& layoutAfter) {
    events.push(event);
    layoutsAfter.push(layoutAfter);
}

void TreeMotion::setFinal(const TreeLayout& layout) {
    finalLayoutData = layout;
}

TreeMotion TreeMotion::simple(const TreeLayout& before, const EventLog& source,
                              const TreeLayout& after) {
    TreeMotion motion;
    motion.setBase(before);
    motion.setFinal(after);
    for (int i = 0; i < source.size(); i++) {
        motion.addEvent(source.at(i), after);
    }
    return motion;
}

TreeLayout TreeMotion::frameAt(int activeIndex, float progress) const {
    if (events.empty()) return finalLayoutData;
    if (activeIndex < 0) return baseLayout;
    if (activeIndex >= events.size()) return finalLayoutData;

    if (progress > 1.f) progress = 1.f;
    if (progress < 0.f) progress = 0.f;

    const TreeLayout& before = activeIndex == 0 ? baseLayout : layoutsAfter[activeIndex - 1];
    const TreeLayout& after = layoutsAfter[activeIndex];
    if (!movesPositions(events[activeIndex].type)) {
        return before;
    }
    if (progress >= 1.f) {
        return after;
    }

    DynArray<LayoutPoint> points;
    for (int i = 0; i < after.size(); i++) {
        LayoutPoint point = after.at(i);
        if (before.contains(point.nodeId)) {
            point.x = lerp(before.xOf(point.nodeId), point.x, progress);
            point.y = lerp(before.yOf(point.nodeId), point.y, progress);
        }
        points.push(point);
    }

    LayoutBox box;
    box.minX = lerp(before.bounds().minX, after.bounds().minX, progress);
    box.minY = lerp(before.bounds().minY, after.bounds().minY, progress);
    box.maxX = lerp(before.bounds().maxX, after.bounds().maxX, progress);
    box.maxY = lerp(before.bounds().maxY, after.bounds().maxY, progress);
    return TreeLayout(points, box);
}

}
