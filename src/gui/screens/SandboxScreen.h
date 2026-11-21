#pragma once
#include <string>
#include "gui/core/AnimationController.h"
#include "gui/core/Screen.h"
#include "gui/core/Theme.h"
#include "gui/widgets/HeapView.h"
#include "gui/widgets/ReplayScrubber.h"
#include "model/heap/MaxHeap.h"
#include "model/tree/AVLTree.h"
#include "model/tree/BST.h"
#include "model/tree/BinaryTree.h"

using namespace std;

namespace vh {

class SandboxScreen : public Screen {
public:
    SandboxScreen(Theme& theme);
    int handleEvent(const sf::Event& event, sf::RenderWindow& window) override;
    int update(float seconds) override;
    void draw(sf::RenderWindow& window) override;
    void onShow() override;

private:
    Theme& theme;
    int structure; // 0 Binary, 1 BST, 2 AVL, 3 MaxHeap
    int inputValue;
    string message;
    BinaryTree binary;
    BST bst;
    AVLTree avl;
    MaxHeap heap;
    HeapView heapView;
    AnimationController animation;
    ReplayScrubber scrubber;

    sf::FloatRect structureButton() const;
    sf::FloatRect insertButton() const;
    sf::FloatRect removeButton() const;
    sf::FloatRect randomButton() const;
    sf::FloatRect clearButton() const;
    sf::FloatRect buildButton() const;
    sf::FloatRect backButton() const;

    string structureName() const;
    void cycleStructure();
    void doInsert();
    void doRemove();
    void doRandom();
    void doClear();
    void doBuild();
    void refreshHeapView();
};

}
