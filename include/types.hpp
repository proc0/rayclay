#pragma once

#include <raylib.h>

namespace Action {
    enum Display {
        DO_NOTHING,
        NEW_GAME,
        LOAD_GAME,
        OPTIONS,
        RESUME_GAME,
        MAIN_MENU,
        CONFIRM_RETURN_MAIN,
        CANCEL_RETURN_MAIN,
        QUIT_APP,
    };
}

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
        LOAD,
        RUN,
        PAUSE,
        HALT
    };

    enum AppScreen {
        INTRO,
        TITLE,
        MAIN,
        GAME
    };

    enum Game {
        START,
        PLAY,
        HOLD,
        OVER,
        END
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

typedef struct GameState {
    int raylibLogoClicks;
    int raylibLogoBounces;
    int raylibLogoCorners;
} GameState;

// typedef struct TimerEvent {
//     uint16_t id;
//     Event::Timer state;
// } TimerEvent;

class Layer {
public:
    virtual ~Layer() = default;
    virtual void resize(int width, int height) = 0;
};