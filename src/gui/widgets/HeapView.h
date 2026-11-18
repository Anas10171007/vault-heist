#pragma once
#include <SFML/System/Vector2.hpp>
#include "gui/core/AnimationController.h"
#include "util/DynArray.h"

class Renderer;

namespace vh {

class HeapView : public AnimationListener {
public:
    HeapView();

    void load(const DynArray<int>& startValues, int startHeapSize);
    void reset() override;
    void onEvent(const Event& event, float progress) override;

    void draw(Renderer& renderer, sf::Vector2f treeOrigin, float treeWidth,
              sf::Vector2f arrayOrigin, float arrayWidth) const;

    int valueCount() const;
    int activeSize() const;
    int valueAt(int index) const;
    bool isViolation(int index) const;
    int swappingFirst() const;
    int swappingSecond() const;

private:
    DynArray<int> initialValues;
    DynArray<int> values;
    int initialHeapSize;
    int heapSize;
    int swapA;
    int swapB;
    float swapProgress;

    sf::Vector2f treePosition(int index, sf::Vector2f origin, float width) const;
    sf::Vector2f arrayPosition(int index, sf::Vector2f origin,
                              int cellsPerRow, float gap) const;
    sf::Vector2f movingPosition(int index, sf::Vector2f normal,
                               sf::Vector2f first, sf::Vector2f second) const;
};

}
