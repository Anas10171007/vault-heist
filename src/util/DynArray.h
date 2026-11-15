#pragma once

using namespace std;

namespace vh {

template <typename T>
class DynArray {
public:
    DynArray() {
        data = nullptr;
        count = 0;
        capacity = 0;
    }

    DynArray(const DynArray& other) {
        count = other.count;
        capacity = other.capacity;
        data = capacity == 0 ? nullptr : new T[capacity];

        for (int i = 0; i < count; i++) {
            data[i] = other.data[i];
        }
    }

    DynArray& operator=(const DynArray& other) {
        if (this == &other) {
            return *this;
        }

        T* newData = other.capacity == 0 ? nullptr : new T[other.capacity];
        for (int i = 0; i < other.count; i++) {
            newData[i] = other.data[i];
        }

        delete[] data;
        data = newData;
        count = other.count;
        capacity = other.capacity;
        return *this;
    }

    ~DynArray() {
        delete[] data;
    }

    void push(const T& value) {
        T valueCopy = value;

        if (count == capacity) {
            grow();
        }

        data[count] = valueCopy;
        count++;
    }

    T pop() {
        if (empty()) {
            return T();
        }

        count--;
        T removedValue = data[count];
        data[count] = T();
        return removedValue;
    }

    T& at(int index) {
        return data[index];
    }

    const T& at(int index) const {
        return data[index];
    }

    T& operator[](int index) {
        return at(index);
    }

    const T& operator[](int index) const {
        return at(index);
    }

    int size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }

    void clear() {
        while (count > 0) {
            count--;
            data[count] = T();
        }
    }

private:
    T* data;
    int count;
    int capacity;

    void grow() {
        int newCapacity = capacity == 0 ? 4 : capacity * 2;
        T* newData = new T[newCapacity];

        for (int i = 0; i < count; i++) {
            newData[i] = data[i];
        }

        delete[] data;
        data = newData;
        capacity = newCapacity;
    }
};

}
