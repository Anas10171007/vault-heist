#include "gui/widgets/HeapView.h"
#include "gui/core/Renderer.h"
#include "gui/core/Style.h"
#include <string>

using namespace std;

namespace vh {

HeapView::HeapView()
    : initialHeapSize(0), heapSize(0), swapA(-1), swapB(-1), swapProgress(0),
      selected(-1), sealIndex(-1), sealProgress(0) {}

void HeapView::load(const DynArray<int>& startValues, int startHeapSize) {
    initialValues = startValues;
    initialHeapSize = startHeapSize;
    if (initialHeapSize < 0) initialHeapSize = 0;
    if (initialHeapSize > initialValues.size()) initialHeapSize = initialValues.size();
    reset();
}

void HeapView::reset() {
    values = initialValues;
    heapSize = initialHeapSize;
    swapA = -1;
    swapB = -1;
    swapProgress = 0;
    selected = -1;
    sealIndex = -1;
    sealProgress = 0;
}

void HeapView::onEvent(const Event& event, float progress) {
    if (event.type == EventType::INSERT && progress >= 1) {
        if (event.idA == values.size()) values.push(event.key);
    }

    if (event.type == EventType::HEAP_SWAP) {
        swapA = event.idA;
        swapB = event.idB;
        swapProgress = progress;
        if (progress >= 1) {
            if (swapA >= 0 && swapA < values.size() &&
                swapB >= 0 && swapB < values.size()) {
                int temporary = values[swapA];
                values[swapA] = values[swapB];
                values[swapB] = temporary;
            }
            swapA = -1;
            swapB = -1;
            swapProgress = 0;
        }
    }

    // Shrink the active heap immediately, then fly the sealed leaf into SORTED
    // so it does not vanish from the tree with no destination.
    if (event.type == EventType::HEAP_SIZE_CHANGE) {
        int nextSize = event.newValue;
        if (nextSize < 0) nextSize = 0;
        if (nextSize > values.size()) nextSize = values.size();

        int fromSize = event.oldValue;
        if (fromSize < 1) fromSize = heapSize;
        if (fromSize > values.size()) fromSize = values.size();

        if (sealIndex < 0 && nextSize < fromSize) {
            sealIndex = fromSize - 1;
        }
        heapSize = nextSize;
        sealProgress = progress;
        if (progress >= 1) {
            sealIndex = -1;
            sealProgress = 0;
        }
    }
}

sf::Vector2f HeapView::treePosition(int index, sf::Vector2f origin, float width) const {
    int first = 0;
    int count = 1;
    int level = 0;
    while (index >= first + count) {
        first += count;
        count *= 2;
        level++;
    }

    int place = index - first;
    float x = origin.x + (static_cast<float>(place) + 0.5f) *
              width / static_cast<float>(count);
    float y = origin.y + static_cast<float>(level) * 90.f;
    return {x, y};
}

sf::Vector2f HeapView::arrayPosition(int index, sf::Vector2f origin,
                                    int cellsPerRow, float gap) const {
    int row = index / cellsPerRow;
    int column = index % cellsPerRow;
    return {origin.x + Renderer::CELL_W / 2.f +
            static_cast<float>(column) * (Renderer::CELL_W + gap),
            origin.y + Renderer::CELL_H / 2.f + static_cast<float>(row) * 82.f};
}

sf::Vector2f HeapView::movingPosition(int index, sf::Vector2f normal,
                                     sf::Vector2f first, sf::Vector2f second) const {
    if (index != swapA && index != swapB) return normal;
    sf::Vector2f start = index == swapA ? first : second;
    sf::Vector2f finish = index == swapA ? second : first;
    sf::Vector2f position = start + (finish - start) * swapProgress;
    float arc = 28.f * 4.f * swapProgress * (1.f - swapProgress);
    position.y += index == swapA ? -arc : arc;
    return position;
}

bool HeapView::isViolation(int index) const {
    if (index < 0 || index >= heapSize) return false;

    if (index > 0) {
        int parent = (index - 1) / 2;
        if (values[parent] < values[index]) return true;
    }

    int left = index * 2 + 1;
    int right = index * 2 + 2;
    if (left < heapSize && values[index] < values[left]) return true;
    if (right < heapSize && values[index] < values[right]) return true;
    return false;
}

void HeapView::draw(Renderer& renderer, sf::Vector2f treeOrigin, float treeWidth,
                    sf::Vector2f arrayOrigin, float arrayWidth) const {
    renderer.drawText("HEAP TREE", {treeOrigin.x, treeOrigin.y - 28.f},
                      13, Palette::MUTED, false);
    renderer.drawText("size " + to_string(heapSize),
                      {treeOrigin.x + 110.f, treeOrigin.y - 28.f},
                      13, Palette::MUTED, false);

    float gap = 8.f;
    int cellsPerRow = static_cast<int>((arrayWidth + gap) /
                                      (Renderer::CELL_W + gap));
    if (cellsPerRow < 1) cellsPerRow = 1;

    sf::Vector2f treeFirst;
    sf::Vector2f treeSecond;
    if (swapA >= 0 && swapA < heapSize && swapB >= 0 && swapB < heapSize) {
        treeFirst = treePosition(swapA, treeOrigin, treeWidth);
        treeSecond = treePosition(swapB, treeOrigin, treeWidth);
    }

    for (int index = 1; index < heapSize; index++) {
        int parent = (index - 1) / 2;
        bool highlight = index == swapA || index == swapB ||
                         isViolation(index) || isViolation(parent);
        renderer.drawEdge(treePosition(parent, treeOrigin, treeWidth),
                          treePosition(index, treeOrigin, treeWidth), highlight);
    }

    for (int index = 0; index < heapSize; index++) {
        sf::Vector2f normal = treePosition(index, treeOrigin, treeWidth);
        sf::Vector2f position = movingPosition(index, normal, treeFirst, treeSecond);
        int state = NodeState::NORMAL;
        if (index == selected) state = NodeState::TARGET;
        else if (isViolation(index)) state = NodeState::VIOLATION;
        else if (index == swapA || index == swapB) state = NodeState::VISITED;
        renderer.drawNode(position, to_string(values[index]), state);
        if (index == 0) {
            renderer.drawBadge(position + sf::Vector2f(34.f, -30.f),
                               "MAX", NodeState::TARGET);
        }
    }

    renderer.drawText("ARRAY", {arrayOrigin.x, arrayOrigin.y - 28.f},
                      13, Palette::MUTED, false);

    sf::Vector2f arrayFirst;
    sf::Vector2f arraySecond;
    if (swapA >= 0 && swapA < values.size() && swapB >= 0 && swapB < values.size()) {
        arrayFirst = arrayPosition(swapA, arrayOrigin, cellsPerRow, gap);
        arraySecond = arrayPosition(swapB, arrayOrigin, cellsPerRow, gap);
    }

    bool sealing = sealIndex >= 0 && sealIndex < values.size() && sealProgress < 1.f;
    int previousRegion = -1;
    int previousRow = -1;
    for (int index = 0; index < values.size(); index++) {
        int row = index / cellsPerRow;
        int region = index < heapSize ? CellRegion::ACTIVE : CellRegion::SORTED;
        if (row != previousRow || region != previousRegion) {
            sf::Vector2f labelPosition = arrayPosition(index, arrayOrigin,
                                                       cellsPerRow, gap);
            renderer.drawText(region == CellRegion::ACTIVE ? "ACTIVE HEAP" : "SORTED",
                              {labelPosition.x, labelPosition.y - 36.f},
                              11, Palette::MUTED, true);
            previousRow = row;
            previousRegion = region;
        }

        // Hide the destination cell until the flying max nearly lands.
        if (sealing && index == sealIndex && sealProgress < 0.85f) continue;

        sf::Vector2f normal = arrayPosition(index, arrayOrigin, cellsPerRow, gap);
        sf::Vector2f position = movingPosition(index, normal, arrayFirst, arraySecond);
        if (index == selected) region = CellRegion::SELECTED;
        if (index == swapA || index == swapB) region = CellRegion::SWAPPING;
        if (index == sealIndex) region = CellRegion::SORTED;
        renderer.drawArrayCell(position, index, values[index], region);
    }

    // Fly the sealed max from its tree leaf into the SORTED array slot.
    if (sealIndex >= 0 && sealIndex < values.size()) {
        sf::Vector2f from = treePosition(sealIndex, treeOrigin, treeWidth);
        sf::Vector2f to = arrayPosition(sealIndex, arrayOrigin, cellsPerRow, gap);
        float t = sealProgress;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        sf::Vector2f pos = from + (to - from) * t;
        float arc = 40.f * 4.f * t * (1.f - t);
        pos.y += arc;
        renderer.drawNode(pos, to_string(values[sealIndex]), NodeState::TARGET);
        renderer.drawBadge(pos + sf::Vector2f(34.f, -30.f), "MAX", NodeState::TARGET);
    }
}

int HeapView::valueCount() const { return values.size(); }
int HeapView::activeSize() const { return heapSize; }
int HeapView::valueAt(int index) const { return values[index]; }
int HeapView::swappingFirst() const { return swapA; }
int HeapView::swappingSecond() const { return swapB; }

void HeapView::setSelectedIndex(int index) {
    if (index >= 0 && index < heapSize) selected = index;
    else selected = -1;
}

int HeapView::selectedIndex() const { return selected; }

int HeapView::hitTest(sf::Vector2f point, sf::Vector2f treeOrigin, float treeWidth,
                      sf::Vector2f arrayOrigin, float arrayWidth) const {
    float gap = 8.f;
    int cellsPerRow = static_cast<int>((arrayWidth + gap) /
                                      (Renderer::CELL_W + gap));
    if (cellsPerRow < 1) cellsPerRow = 1;

    for (int index = 0; index < heapSize; index++) {
        sf::Vector2f cell = arrayPosition(index, arrayOrigin, cellsPerRow, gap);
        if (point.x >= cell.x - Renderer::CELL_W / 2.f &&
            point.x <= cell.x + Renderer::CELL_W / 2.f &&
            point.y >= cell.y - Renderer::CELL_H / 2.f &&
            point.y <= cell.y + Renderer::CELL_H / 2.f) {
            return index;
        }
    }

    for (int index = 0; index < heapSize; index++) {
        sf::Vector2f node = treePosition(index, treeOrigin, treeWidth);
        if ((point - node).length() <= Renderer::NODE_HIT_RADIUS) return index;
    }
    return -1;
}

sf::Vector2f HeapView::sortedDropCenter(sf::Vector2f arrayOrigin,
                                        float arrayWidth) const {
    float gap = 8.f;
    int cellsPerRow = static_cast<int>((arrayWidth + gap) /
                                      (Renderer::CELL_W + gap));
    if (cellsPerRow < 1) cellsPerRow = 1;
    return arrayPosition(heapSize, arrayOrigin, cellsPerRow, gap);
}

bool HeapView::hitsSortedDrop(sf::Vector2f point, sf::Vector2f arrayOrigin,
                              float arrayWidth) const {
    float gap = 8.f;
    int cellsPerRow = static_cast<int>((arrayWidth + gap) /
                                      (Renderer::CELL_W + gap));
    if (cellsPerRow < 1) cellsPerRow = 1;

    for (int index = heapSize; index < values.size(); index++) {
        sf::Vector2f cell = arrayPosition(index, arrayOrigin, cellsPerRow, gap);
        if (point.x >= cell.x - Renderer::CELL_W / 2.f &&
            point.x <= cell.x + Renderer::CELL_W / 2.f &&
            point.y >= cell.y - Renderer::CELL_H / 2.f &&
            point.y <= cell.y + Renderer::CELL_H / 2.f) {
            return true;
        }
    }

    // Next slot after the active region (where the extracted max lands).
    sf::Vector2f drop = arrayPosition(heapSize, arrayOrigin, cellsPerRow, gap);
    float pad = 18.f;
    if (point.x >= drop.x - Renderer::CELL_W / 2.f - pad &&
        point.x <= drop.x + Renderer::CELL_W / 2.f + pad &&
        point.y >= drop.y - Renderer::CELL_H / 2.f - pad &&
        point.y <= drop.y + Renderer::CELL_H / 2.f + pad) {
        return true;
    }
    return false;
}

}
