#pragma once

#include "raylib.h"

namespace Action {
    enum Surface {
        DO_NOTHING,
        NEW_GAME,
        CONFIRM_TUTORIAL,
        LOAD_GAME,
        OPTIONS,
        RESUME_GAME,
        MAIN_MENU,
        CONFIRM_RETURN,
        CANCEL_RETURN,
        CONFIRM_OPTIONS,
        CANCEL_OPTIONS,
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

    enum Surface {
        NO_EVENT,
        SHOW_TUTORIAL,
        CONFIRM_TUTORIAL,
        SHOW_RETURN_MAIN_MENU_CONFIRMATION,
        SHOW_OPTIONS,
    };
}

namespace State {
    enum App {
        LOAD,
        RUN,
        HOLD,
        HALT
    };

    enum Screen {
        INTRO,
        TITLE,
        MAIN,
        GAME
    };

    enum Game {
        START,
        PLAY,
        PAUSE,
        OVER,
        FINISH
    };

    enum Pointer {
        IDLE,
        GRAB,
        DRAG,
        DROP
    };
}

struct InputEvent {
    Event::Input id;
    Vector2 position;
};

#define DEFAULT_INPUT InputEvent({ .id = Event::Input::IDLE, .position = Vector2({}) })

struct GameState {
    // int raylibLogoClicks;
    // int raylibLogoBounces;
    // int raylibLogoCorners;
    State::Game state;
};

class Layer {
public:
    virtual ~Layer() = default;
    virtual void resize(int width, int height) {};
};
