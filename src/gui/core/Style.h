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

struct ButtonKind {
    static constexpr int PRIMARY = 0;
    static constexpr int SECONDARY = 1;
    static constexpr int GHOST = 2;
    static constexpr int DANGER = 3;
};

// Vault noir: deep charcoal + brass + teal. No purple, no flat navy slab.
struct Palette {
    static constexpr sf::Color BG{9, 11, 15};
    static constexpr sf::Color BG_TOP{16, 20, 28};
    static constexpr sf::Color PANEL{18, 23, 33};
    static constexpr sf::Color PANEL_HI{24, 30, 42};
    static constexpr sf::Color BORDER{48, 58, 74};
    static constexpr sf::Color BORDER_HI{78, 92, 112};
    static constexpr sf::Color TEXT{236, 232, 224};
    static constexpr sf::Color MUTED{118, 126, 140};
    static constexpr sf::Color NODE_FILL{30, 38, 54};
    static constexpr sf::Color NODE_STROKE{150, 164, 188};
    static constexpr sf::Color EDGE{88, 102, 128};
    static constexpr sf::Color INK{10, 12, 16};
    static constexpr sf::Color CHIP{12, 14, 20, 210};
    static constexpr sf::Color BALANCED{62, 176, 118};
    static constexpr sf::Color WARNING{232, 176, 64};
    static constexpr sf::Color VIOLATION{220, 64, 72};
    static constexpr sf::Color VISITED{64, 178, 168};
    static constexpr sf::Color SKIPPED{108, 118, 134};
    static constexpr sf::Color TARGET{196, 155, 74};
    static constexpr sf::Color BRASS{212, 170, 84};
    static constexpr sf::Color SCAN{64, 178, 168, 28};

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
