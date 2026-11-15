#include <iostream>
#include <string>
#include "util/DynArray.h"
#include "util/Stack.h"
#include "util/Queue.h"
#include "util/Rng.h"

int runUtilityTests() {
    using namespace vh;
    using namespace std;
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << '\n';
        if (!ok) ++failed;
    };
    DynArray<int> array;
    check(array.empty() && array.size() == 0, "array starts empty");
    check(array.pop() == 0 && array.empty(), "empty array pop returns a default value");
    for (int i = 0; i < 1000; ++i) array.push(i);
    bool ordered = true;
    for (int i = 0; i < 1000; ++i) ordered = ordered && array.at(i) == i;
    check(ordered && array.size() == 1000, "array growth preserves 1000 elements");
    DynArray<int> copy(array);
    copy[0] = -1;
    DynArray<int> assigned;
    assigned.push(-10);
    assigned = array;
    assigned[1] = -2;
    array = array;
    check(array[0] == 0 && array[1] == 1 && copy[0] == -1 && assigned[1] == -2, "array copies own their storage and self assignment works");
    check(array.pop() == 999 && array.size() == 999, "array pop returns last element");
    array.clear();
    array.push(42);
    check(array.size() == 1 && array[0] == 42, "array clear permits reuse");
    DynArray<string> words;
    for (int i = 0; i < 4; ++i) words.push("treasure");
    words.push(words[0]);
    const DynArray<string>& constWords = words;
    check(constWords.at(4) == "treasure", "array handles strings and self-referencing growth");

    Stack<int> stack;
    check(stack.empty() && stack.pop() == 0, "empty stack pop returns a default value");
    for (int i = 0; i < 1000; ++i) stack.push(i);
    Stack<int> stackCopy(stack);
    const Stack<int>& constStack = stack;
    check(constStack.top() == 999 && stack.size() == 1000, "stack top observes last item");
    bool lifo = true;
    for (int i = 999; i >= 0; --i) lifo = lifo && stack.pop() == i;
    check(lifo && stack.empty() && stackCopy.size() == 1000, "stack is LIFO and copies independently");
    stackCopy.clear();
    check(stackCopy.empty(), "stack clear empties storage");

    Queue<int> queue;
    check(queue.empty() && queue.dequeue() == 0, "empty queue dequeue returns a default value");
    for (int i = 0; i < 500; ++i) queue.enqueue(i);
    bool fifo = true;
    for (int i = 0; i < 200; ++i) fifo = fifo && queue.dequeue() == i;
    for (int i = 500; i < 1000; ++i) queue.enqueue(i);
    Queue<int> queueCopy(queue);
    Queue<int> queueAssigned;
    queueAssigned = queue;
    const Queue<int>& constQueue = queue;
    check(constQueue.front() == 200 && queue.size() == 800, "queue front and size work with both stacks occupied");
    for (int i = 200; i < 1000; ++i) fifo = fifo && queue.dequeue() == i;
    check(fifo && queue.empty() && queueCopy.dequeue() == 200 && queueAssigned.dequeue() == 200, "queue is FIFO through transfers and copies independently");
    queueCopy.clear();
    queueCopy.enqueue(9);
    check(queueCopy.dequeue() == 9 && queueCopy.empty(), "queue clear permits reuse");

    Rng rng(1);
    const unsigned int expected[] = {1015568748u, 1586005467u, 2165703038u, 3027450565u, 217083232u};
    bool sequence = true;
    for (auto value : expected) sequence = sequence && rng.next() == value;
    check(sequence, "RNG seed 1 matches fixed cross-platform sequence");
    Rng zero(0);
    check(zero.next() == 1013904223u, "RNG accepts zero seed");
    Rng sameA(42), sameB(42);
    bool repeatable = true;
    bool bounded = true;
    for (int i = 0; i < 10000; ++i) {
        const int a = sameA.range(-7, 9);
        repeatable = repeatable && a == sameB.range(-7, 9);
        bounded = bounded && a >= -7 && a <= 9;
    }
    check(repeatable && bounded, "RNG ranges repeat and stay within inclusive bounds");
    check(rng.range(5, 5) == 5, "RNG supports singleton range");
    int reversedValue = rng.range(3, 2);
    check(reversedValue >= 2 && reversedValue <= 3, "RNG handles reversed range bounds");
    DynArray<int> shuffled;
    for (int i = 0; i < 10; ++i) shuffled.push(i);
    DynArray<int> shuffledAgain(shuffled);
    Rng shuffleA(123), shuffleB(123);
    shuffleA.shuffle(shuffled);
    shuffleB.shuffle(shuffledAgain);
    bool seen[10] = {};
    bool permutation = true;
    bool sameShuffle = true;
    for (int i = 0; i < shuffled.size(); ++i) {
        int value = shuffled[i];
        if (value < 0 || value >= 10 || seen[value]) permutation = false;
        else seen[value] = true;
        sameShuffle = sameShuffle && value == shuffledAgain[i];
    }
    check(permutation && sameShuffle, "shuffle preserves permutation and repeats with same seed");
    shuffled.clear();
    shuffleA.shuffle(shuffled);
    shuffled.push(7);
    shuffleA.shuffle(shuffled);
    check(shuffled.size() == 1 && shuffled[0] == 7, "shuffle handles empty and singleton arrays");
    return failed;
}
