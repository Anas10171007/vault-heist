#pragma once
#include <string>
#include "model/Event.h"

using namespace std;

namespace vh {

inline string describeEvent(const Event& event) {
    string line = eventName(event.type);
    if (event.type == EventType::HEAP_SWAP || event.type == EventType::SWAP_VALUE) {
        line += "  [" + to_string(event.idA) + "] <-> [" + to_string(event.idB) + "]";
        if (event.oldValue != 0 || event.newValue != 0) {
            line += "  (" + to_string(event.oldValue) + " / " + to_string(event.newValue) + ")";
        }
        return line;
    }
    if (event.type == EventType::EXTRACT_MAX) {
        line += "  value " + to_string(event.key);
        return line;
    }
    if (event.type == EventType::ROTATE) {
        line += "  pivot " + to_string(event.idA);
        return line;
    }
    if (event.type == EventType::VISIT || event.type == EventType::COMPARE ||
        event.type == EventType::INSERT || event.type == EventType::UNLINK) {
        line += "  id " + to_string(event.idA);
        if (event.key != 0) line += "  key " + to_string(event.key);
        return line;
    }
    if (event.idA != 0 || event.idB != 0) {
        line += "  a=" + to_string(event.idA) + " b=" + to_string(event.idB);
    }
    if (event.key != 0) line += "  key=" + to_string(event.key);
    return line;
}

}
