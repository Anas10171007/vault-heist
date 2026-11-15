#pragma once
#include "util/DynArray.h"
using namespace std;

namespace vh {

template <typename T>
class Stack {
public:
    void push(const T& value) { items.push(value); }
    T pop() { return items.pop(); }
    T& top() { return items.at(items.size() - 1); }
    const T& top() const { return items.at(items.size() - 1); }
    int size() const { return items.size(); }
    bool empty() const { return items.empty(); }
    void clear() { items.clear(); }
private:
    DynArray<T> items;
};

}
