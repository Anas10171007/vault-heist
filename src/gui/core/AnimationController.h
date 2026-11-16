#pragma once
#include "model/EventLog.h"

namespace vh {

// The renderer implements these functions. Neither function changes the model.
class AnimationListener {
public:
    virtual ~AnimationListener() {}
    virtual void reset() = 0;
    virtual void onEvent(const Event& event, float progress) = 0;
};

class AnimationController {
public:
    AnimationController() {
        listener = nullptr;
        completed = 0;
        ticks = 0;
        accumulated = 0;
        speed = 1;
        playing = false;
    }

    void setListener(AnimationListener* receiver) {
        listener = receiver;
        rebuild();
    }

    void load(const EventLog& source) {
        events.clear();
        events.extend(source);
        restart();
    }

    void restart() {
        playing = false;
        completed = 0;
        ticks = 0;
        accumulated = 0;
        rebuild();
    }

    void play() { playing = completed < events.size(); }
    void pause() { playing = false; }
    bool isPlaying() const { return playing; }
    int completedCount() const { return completed; }
    int eventCount() const { return events.size(); }
    double getSpeed() const { return speed; }

    void setSpeed(double value) {
        if (value >= 0.25 && value <= 4) speed = value;
    }

    // Smoothstep: slow at the start/end, quicker in the middle.
    static float ease(float t) {
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        return t * t * (3 - 2 * t);
    }

    void update(double seconds) {
        if (!playing || seconds <= 0) return;
        accumulated += seconds * speed;
        const double fixedStep = 1.0 / 60.0;
        while (playing && accumulated + 0.000000001 >= fixedStep) {
            accumulated -= fixedStep;
            if (accumulated < 0) accumulated = 0;
            ticks++;
            if (listener != nullptr) {
                listener->onEvent(events.at(completed), ease(ticks / 60.f));
            }
            // Each event lasts exactly 60 simulation ticks at normal speed.
            if (ticks == 60) {
                completed++;
                ticks = 0;
                if (completed == events.size()) {
                    playing = false;
                    accumulated = 0;
                }
            }
        }
    }

    void stepForward() {
        pause();
        if (completed < events.size()) completed++;
        ticks = 0;
        accumulated = 0;
        rebuild();
    }

    void stepBackward() {
        pause();
        // During an event, Left returns to its start; otherwise undo one event.
        if (ticks == 0 && completed > 0) completed--;
        ticks = 0;
        accumulated = 0;
        rebuild();
    }

    void finishInstantly() {
        pause();
        completed = events.size();
        ticks = 0;
        accumulated = 0;
        rebuild();
    }

private:
    EventLog events;
    AnimationListener* listener;
    int completed;
    int ticks;
    double accumulated;
    double speed;
    bool playing;

    void rebuild() {
        if (listener == nullptr) return;
        listener->reset();
        for (int i = 0; i < completed; i++) listener->onEvent(events.at(i), 1);
        if (ticks > 0 && completed < events.size()) {
            listener->onEvent(events.at(completed), ease(ticks / 60.f));
        }
    }
};

}
