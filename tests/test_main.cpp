#include <iostream>
#include <string>
#include "model/Node.h"
#include "model/NodeId.h"
#include "model/EventLog.h"
int main() {
    using namespace vh;
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        std::cout << (ok ? "PASS " : "FAIL ") << name << std::endl;
        if (!ok) {
            failed++;
        }
    };
    IdSource ids;
    TreeNode a(ids.next(), 10);
    TreeNode b(ids.next(), 20);
    check(a.id == 1 && b.id == 2, "ids start at 1 and increase");
    check(a.left == nullptr && a.right == nullptr && a.parent == nullptr, "links start null");
    BSTNode c(ids.next(), 50, 7);
    check(c.id == 3 && c.key == 50 && c.value == 50 && c.treasure == 7, "BSTNode keeps value equal to key");
    c.setKey(60);
    check(c.key == 60 && c.value == 60, "setKey updates key and value");
    AVLNode d(ids.next(), 40);
    check(d.height == 1 && d.size == 1 && d.id == 4, "AVLNode starts as a leaf");
    TreeNode* base = new AVLNode(ids.next(), 30);
    check(base->id == 5, "base pointer sees derived id");
    delete base;
    ids.reserve(99);
    AVLNode e(ids.next(), 15);
    check(e.id == 100, "reserve moves the counter forward");
    IdSource other;
    check(other.next() == 1, "each source counts on its own");
    int last = e.id;
    bool increasing = true;
    for (int i = 0; i < 1000; i++) {
        TreeNode n(ids.next(), i);
        if (n.id <= last) {
            increasing = false;
        }
        last = n.id;
    }
    check(increasing, "ids stay strictly increasing over 1000 nodes");
    check(EventType::DONE + 1 == EventType::COUNT, "19 event types");
    EventLog log;
    check(log.empty() && log.size() == 0 && log.isRecording(), "new log is empty and recording");
    log.append(Event(EventType::VISIT, 7));
    log.append(Event(EventType::INSERT, 8, 7, Side::LEFT, 25));
    check(log.size() == 2 && log.at(0).type == EventType::VISIT && log.at(0).idA == 7, "append and read back");
    check(log.at(1).idA == 8 && log.at(1).idB == 7 && log.at(1).side == Side::LEFT && log.at(1).key == 25, "payload fields kept");
    for (int i = 0; i < 100; i++) {
        log.append(Event(EventType::COMPARE, i));
    }
    check(log.size() == 102 && log.at(0).idA == 7 && log.at(101).idA == 99, "growth keeps old events");
    log.setRecordEvents(false);
    log.append(Event(EventType::VISIT, 1));
    check(log.size() == 102, "recording off ignores append");
    log.setRecordEvents(true);
    EventLog second;
    second.extend(log);
    check(second.size() == 102 && second.at(1).key == 25, "extend copies events");
    log.clear();
    check(log.empty(), "clear empties the log");
    EventLog off(false);
    off.append(Event(EventType::DONE));
    check(off.empty() && !off.isRecording(), "log created with recording off stays empty");
    auto build = []() {
        EventLog r;
        r.append(Event(EventType::DONE));
        return r;
    };
    EventLog got = build();
    check(got.size() == 1, "log can be returned by value");
    EventLog target;
    target.append(Event(EventType::VISIT, 1));
    target = build();
    check(target.size() == 1 && target.at(0).type == EventType::DONE, "move assignment replaces the events");
    check(std::string(eventName(EventType::HEAP_SORT_COMPLETE)) == "HEAP_SORT_COMPLETE", "event names print");
    std::cout << (failed == 0 ? "all passed" : "some failed") << std::endl;
    return failed;
}