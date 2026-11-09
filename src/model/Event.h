#pragma once
namespace vh {
struct EventType {
    static constexpr int VISIT = 0, COMPARE = 1, INSERT = 2, UNLINK = 3, SWAP_VALUE = 4, ROTATE = 5, UPDATE_HEIGHT = 6;
    static constexpr int UPDATE_SIZE = 7, BALANCE_ALERT = 8, HIGHLIGHT_SUBTREE = 9, HEAP_BUILD = 10, HEAP_SWAP = 11;
    static constexpr int HEAPIFY_START = 12, HEAPIFY_DONE = 13, EXTRACT_MAX = 14, MARK_SORTED = 15;
    static constexpr int HEAP_SIZE_CHANGE = 16, HEAP_SORT_COMPLETE = 17, DONE = 18, COUNT = 19;
};
struct Side {
    static constexpr int NONE = 0, LEFT = 1, RIGHT = 2;
};
struct RotationType {
    static constexpr int LL = 0, RR = 1, LR = 2, RL = 3;
};
struct HighlightMode {
    static constexpr int VISITED = 0, SKIPPED = 1, RESULT = 2;
};
struct HeapReason {
    static constexpr int BUILD = 0, SIFT_UP = 1, SIFT_DOWN = 2, EXTRACT = 3, PLAYER = 4;
};
struct Event {
    int type;
    int idA;
    int idB;
    int side;
    int key;
    int oldValue;
    int newValue;
    int extra;
    explicit Event(int t = EventType::DONE, int a = 0, int b = 0, int s = Side::NONE, int k = 0, int ov = 0, int nv = 0, int ex = 0)
        : type(t), idA(a), idB(b), side(s), key(k), oldValue(ov), newValue(nv), extra(ex) {}
};
inline const char* eventName(int t) {
    switch (t) {
    case EventType::VISIT: return "VISIT";
    case EventType::COMPARE: return "COMPARE";
    case EventType::INSERT: return "INSERT";
    case EventType::UNLINK: return "UNLINK";
    case EventType::SWAP_VALUE: return "SWAP_VALUE";
    case EventType::ROTATE: return "ROTATE";
    case EventType::UPDATE_HEIGHT: return "UPDATE_HEIGHT";
    case EventType::UPDATE_SIZE: return "UPDATE_SIZE";
    case EventType::BALANCE_ALERT: return "BALANCE_ALERT";
    case EventType::HIGHLIGHT_SUBTREE: return "HIGHLIGHT_SUBTREE";
    case EventType::HEAP_BUILD: return "HEAP_BUILD";
    case EventType::HEAP_SWAP: return "HEAP_SWAP";
    case EventType::HEAPIFY_START: return "HEAPIFY_START";
    case EventType::HEAPIFY_DONE: return "HEAPIFY_DONE";
    case EventType::EXTRACT_MAX: return "EXTRACT_MAX";
    case EventType::MARK_SORTED: return "MARK_SORTED";
    case EventType::HEAP_SIZE_CHANGE: return "HEAP_SIZE_CHANGE";
    case EventType::HEAP_SORT_COMPLETE: return "HEAP_SORT_COMPLETE";
    case EventType::DONE: return "DONE";
    default: return "UNKNOWN";
    }
}
}