#pragma once
namespace vh {
inline int& idCounter() {
    static int counter = 0;
    return counter;
}
inline int nextNodeId() {
    return ++idCounter();
}
inline void reserveNodeId(int id) {
    if (id > idCounter()) {
        idCounter() = id;
    }
}
inline void resetNodeIds() {
    idCounter() = 0;
}
}