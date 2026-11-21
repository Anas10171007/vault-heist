#pragma once

#include "game/tree/TreeLayout.h"
#include "model/EventLog.h"
#include "util/DynArray.h"

namespace vh {

// Pure position interpolation. No SFML and no model pointers: every frame is
// rebuilt from the layouts recorded in the script, so a replay, a skip to the
// end or an undo can never leave a node at a stale position.
class TreeMotion {
public:
    void reset();
    void setBase(const TreeLayout& layout);
    void addEvent(const Event& event, const TreeLayout& layoutAfter);
    void setFinal(const TreeLayout& layout);

    static TreeMotion simple(const TreeLayout& before, const EventLog& events,
                             const TreeLayout& after);

    int eventCount() const { return events.size(); }
    const Event& eventAt(int index) const { return events[index]; }
    const TreeLayout& finalLayout() const { return finalLayoutData; }

    // activeIndex is the index of the event currently being animated.
    // A negative index means "before the first event".
    TreeLayout frameAt(int activeIndex, float progress) const;

private:
    TreeLayout baseLayout;
    TreeLayout finalLayoutData;
    DynArray<Event> events;
    DynArray<TreeLayout> layoutsAfter;
};

}
