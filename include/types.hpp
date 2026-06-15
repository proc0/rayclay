#pragma once

#include <cstdint>
#include <raylib.h>

namespace Event {
    enum Input {
        IDLE,
        PRIMARY,
        PRIMARY_DOWN,
        PRIMARY_UP,
        SECONDARY,
        SECONDARY_DOWN,
        SECONDARY_UP,
        ZOOM_IN,
        ZOOM_OUT,
        SWIPE_UP,
        SWIPE_DOWN,
        SWIPE_RIGHT,
        SWIPE_LEFT,
        KEY_ESCAPE,
        KEY_OTHER
    };

    enum Timer {
        READY,
        RUNNING,
        FINISHED
    };
}

namespace State {
    enum App {
        NIL,
        RUN,
        END
    };

    enum Game {
        STALL,
        START,
        PLAY,
        PAUSE,
        OVER
    };

    enum Pointer {
        IDLE,
        GRAB,
        DRAG,
        DROP
    };
}

typedef struct InputEvent {
    Event::Input id;
    Vector2 position;
} InputEvent;

// typedef struct TimerEvent {
//     uint16_t id;
//     Event::Timer state;
// } TimerEvent;

class ScreenListener {
public:
    virtual ~ScreenListener() = default;
    virtual void onScreenResize(int width, int height) = 0;
};