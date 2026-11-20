#pragma once
#include <SFML/Graphics/Color.hpp>
struct NodeState {
    static constexpr int NORMAL = 0;
    static constexpr int VISITED = 1;
    static constexpr int SKIPPED = 2;
    static constexpr int TARGET = 3;
    static constexpr int WARNING = 4;
    static constexpr int VIOLATION = 5;
    static constexpr int BALANCED = 6;
    static constexpr int COUNT = 7;
        static const char* tag(int state) {
        switch (state) {
        case VISITED: return "VISIT";
        case SKIPPED: return "SKIP";
        case TARGET: return "GOAL";
        case WARNING: return "WARN";
        case VIOLATION: return "BAD";
        case BALANCED: return "OK";
        default: return "";
        }
    }
};
struct CellRegion {
    static constexpr int ACTIVE = 0;
    static constexpr int SORTED = 1;
    static constexpr int SWAPPING = 2;
    static constexpr int SELECTED = 3;
    static constexpr int COUNT = 4;
};
struct Palette {
    static constexpr sf::Color BG{14, 19, 32};
    static constexpr sf::Color PANEL{24, 32, 51};
    static constexpr sf::Color BORDER{42, 53, 80};
    static constexpr sf::Color TEXT{230, 234, 242};
    static constexpr sf::Color MUTED{138, 148, 168};
    static constexpr sf::Color NODE_FILL{43, 55, 82};
    static constexpr sf::Color NODE_STROKE{143, 160, 192};
    static constexpr sf::Color EDGE{92, 107, 138};
    static constexpr sf::Color INK{11, 15, 24};
        static constexpr sf::Color CHIP{14, 19, 32, 200};
    static constexpr sf::Color BALANCED{46, 173, 91};
    static constexpr sf::Color WARNING{242, 211, 53};
    static constexpr sf::Color VIOLATION{229, 57, 59};
    static constexpr sf::Color VISITED{59, 139, 235};
    static constexpr sf::Color SKIPPED{122, 131, 148};
    static constexpr sf::Color TARGET{217, 162, 27};
    static sf::Color stateColor(int state) {
        switch (state) {
        case NodeState::VISITED: return VISITED;
        case NodeState::SKIPPED: return SKIPPED;
        case NodeState::TARGET: return TARGET;
        case NodeState::WARNING: return WARNING;
        case NodeState::VIOLATION: return VIOLATION;
        case NodeState::BALANCED: return BALANCED;
        default: return NODE_FILL;
        }
    }
};
