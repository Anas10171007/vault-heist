#include <iostream>
#include "model/Node.h"
int main() {
    using namespace vh;
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        std::cout << (ok ? "PASS " : "FAIL ") << name << std::endl;
        if (!ok) {
            failed++;
        }
    };
    resetNodeIds();
    TreeNode a(10);
    TreeNode b(20);
    check(a.id == 1 && b.id == 2, "ids start at 1 and increase");
    check(a.left == nullptr && a.right == nullptr && a.parent == nullptr, "links start null");
    BSTNode c(50, 7);
    check(c.id == 3 && c.key == 50 && c.value == 50 && c.treasure == 7, "BSTNode keeps value equal to key");
    c.setKey(60);
    check(c.key == 60 && c.value == 60, "setKey updates key and value");
    AVLNode d(40);
    check(d.height == 1 && d.size == 1 && d.id == 4, "AVLNode starts as a leaf");
    TreeNode* base = new AVLNode(30);
    check(base->id == 5, "base pointer sees derived id");
    delete base;
    AVLNode e(99, 15, 0);
    AVLNode f(16);
    check(e.id == 99 && f.id == 100, "explicit id bumps the counter");
    int last = f.id;
    bool increasing = true;
    for (int i = 0; i < 1000; i++) {
        TreeNode n(i);
        if (n.id <= last) {
            increasing = false;
        }
        last = n.id;
    }
    check(increasing, "ids stay strictly increasing over 1000 nodes");
    std::cout << (failed == 0 ? "all passed" : "some failed") << std::endl;
    return failed;
}