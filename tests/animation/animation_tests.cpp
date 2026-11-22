#include <iostream>
#include "gui/core/AnimationController.h"

using namespace std;
using namespace vh;

class PlaybackProbe : public AnimationListener {
public:
    int calls;
    int finished;
    int order[5];
    float lastProgress;
    void reset() override {
        calls = 0;
        finished = 0;
        lastProgress = 0;
    }
    void onEvent(const Event& event, float progress) override {
        calls++;
        lastProgress = progress;
        if (progress == 1 && finished < 5) order[finished++] = event.idA;
    }
};

int runAnimationTests() {
    int failed = 0;
    auto check = [&](bool ok, const char* name) {
        cout << (ok ? "PASS " : "FAIL ") << name << '\n';
        if (!ok) failed++;
    };
    EventLog log;
    for (int i = 0; i < 5; i++) log.append(Event(EventType::VISIT, i));
    PlaybackProbe probe;
    AnimationController player;
    player.setListener(&probe);
    player.load(log);
    log.clear();
    check(player.eventCount() == 5 && !player.isPlaying(), "animation owns its copied log and starts paused");
    player.play();
    player.update(0.5);
    check(probe.calls == 30 && probe.lastProgress == 0.5f, "fixed ticks interpolate halfway through first event");
    player.pause();
    player.update(2);
    check(probe.calls == 30, "pause freezes playback");
    player.play();
    player.update(0.5);
    check(player.completedCount() == 1 && probe.finished == 1, "resume completes the interrupted event");
    player.stepForward();
    check(player.completedCount() == 2 && probe.finished == 2 && !player.isPlaying(), "forward step completes one event and pauses");
    player.stepBackward();
    check(player.completedCount() == 1 && probe.finished == 1, "backward step rebuilds visual prefix");
    player.play();
    player.update(0.25);
    player.stepBackward();
    check(player.completedCount() == 1 && probe.finished == 1, "backward during animation returns to current event start");
    player.restart();
    player.setSpeed(2);
    player.play();
    player.update(0.5);
    check(player.completedCount() == 1, "double speed finishes an event in half a second");
    player.setSpeed(0);
    check(player.getSpeed() == 2, "invalid speed is ignored");
    player.finishInstantly();
    bool ordered = probe.finished == 5;
    for (int i = 0; i < 5; i++) ordered = ordered && probe.order[i] == i;
    check(ordered && !player.isPlaying(), "instant playback reconstructs every event in order");
    player.stepForward();
    check(player.completedCount() == 5, "forward step stops at end");
    player.restart();
    player.stepBackward();
    check(player.completedCount() == 0 && probe.finished == 0, "restart clears visuals and backward stops at start");
    player.finishInstantly();
    player.seekTo(2);
    check(player.completedCount() == 2 && probe.finished == 2 && !player.isPlaying(),
          "scrubber seek rebuilds a visual prefix without changing the model");
    player.seekTo(99);
    check(player.completedCount() == 5, "scrubber seek clamps to the end");

    EventLog input;
    for (int i = 0; i < 5; i++) input.append(Event(EventType::VISIT, i));
    PlaybackProbe a, b;
    AnimationController smallFrames, largeFrame;
    smallFrames.setListener(&a);
    largeFrame.setListener(&b);
    smallFrames.load(input);
    largeFrame.load(input);
    smallFrames.play();
    largeFrame.play();
    for (int i = 0; i < 300; i++) smallFrames.update(1.0 / 60.0);
    largeFrame.update(5);
    bool matching = a.calls == 300 && b.calls == 300 && a.finished == 5 && b.finished == 5;
    for (int i = 0; i < 5; i++) matching = matching && a.order[i] == b.order[i];
    check(matching && !smallFrames.isPlaying() && !largeFrame.isPlaying(), "frame partitioning preserves ticks, order, and completion");
    player.load(log);
    player.play();
    player.update(1);
    player.finishInstantly();
    check(!player.isPlaying() && probe.calls == 0, "empty logs are safe");
    check(AnimationController::ease(-1) == 0 && AnimationController::ease(2) == 1,
          "easing clamps endpoints");
    return failed;
}
