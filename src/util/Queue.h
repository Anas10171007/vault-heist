#pragma once
#include "util/Stack.h"
using namespace std;

namespace vh {

template <typename T>
class Queue {
public:
    void enqueue(const T& value) { incoming.push(value); }
    T dequeue() { prepareFront(); return outgoing.pop(); }
    T& front() { prepareFront(); return outgoing.top(); }
    const T& front() const { prepareFront(); return outgoing.top(); }
    int size() const { return incoming.size() + outgoing.size(); }
    bool empty() const { return incoming.empty() && outgoing.empty(); }
    void clear() { incoming.clear(); outgoing.clear(); }
private:
    mutable Stack<T> incoming;
    mutable Stack<T> outgoing;
    void prepareFront() const {
        // Transfer only when needed; each element crosses stacks once.
        if (outgoing.empty()) {
            while (!incoming.empty()) {
                outgoing.push(incoming.top());
                incoming.pop();
            }
        }
    }
};

}
