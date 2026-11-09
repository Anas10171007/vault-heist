#pragma once
#include "model/Event.h"
namespace vh {
class EventLog {
public:
    explicit EventLog(bool on = true);
    ~EventLog();
    EventLog(const EventLog&) = delete;
    EventLog& operator=(const EventLog&) = delete;
    EventLog(EventLog&& other) noexcept;
    EventLog& operator=(EventLog&& other) noexcept;
    void append(const Event& e);
    void extend(const EventLog& other);
    const Event& at(int i) const;
    int size() const;
    bool empty() const;
    void clear();
    void setRecordEvents(bool on);
    bool isRecording() const;
private:
    Event* data;
    int count;
    int capacity;
    bool recording;
    void grow();
};
}