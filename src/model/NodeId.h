#pragma once
namespace vh {
struct IdSource {
    int last = 0;
    int next() {
        return ++last;
    }
    void reserve(int id) {
        if (id > last) {
            last = id;
        }
    }
};
}