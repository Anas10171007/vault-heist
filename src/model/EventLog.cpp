#include "model/EventLog.h"
#include <cassert>
namespace vh {
EventLog::EventLog(bool on) : data(nullptr), count(0), capacity(0), recording(on) {}
EventLog::~EventLog() {
    delete[] data;
}
EventLog::EventLog(EventLog&& other) noexcept : data(other.data), count(other.count), capacity(other.capacity), recording(other.recording) {
    other.data = nullptr;
    other.count = 0;
    other.capacity = 0;
}
EventLog& EventLog::operator=(EventLog&& other) noexcept {
    if (this != &other) {
        delete[] data;
        data = other.data;
        count = other.count;
        capacity = other.capacity;
        recording = other.recording;
        other.data = nullptr;
        other.count = 0;
        other.capacity = 0;
    }
    return *this;
}
void EventLog::grow() {
    int newCapacity = capacity == 0 ? 16 : capacity * 2;
    Event* bigger = new Event[newCapacity];
    for (int i = 0; i < count; i++) {
        bigger[i] = data[i];
    }
    delete[] data;
    data = bigger;
    capacity = newCapacity;
}
void EventLog::append(const Event& e) {
    if (!recording) {
        return;
    }
    if (count == capacity) {
        grow();
    }
    data[count++] = e;
}
void EventLog::extend(const EventLog& other) {
    int n = other.count;
    for (int i = 0; i < n; i++) {
        append(other.data[i]);
    }
}
const Event& EventLog::at(int i) const {
    assert(i >= 0 && i < count);
    return data[i];
}
int EventLog::size() const {
    return count;
}
bool EventLog::empty() const {
    return count == 0;
}
void EventLog::clear() {
    count = 0;
}
void EventLog::setRecordEvents(bool on) {
    recording = on;
}
bool EventLog::isRecording() const {
    return recording;
}
}