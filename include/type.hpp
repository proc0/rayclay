#pragma once

#include "timer.hpp"

#include "raylib.h"

namespace Action {
    enum Interface {
        NOTHING,
        MENU_GAME_NEW,
        // MENU_GAME_PAUSE,
        MENU_GAME_RESUME,
        MENU_CONFIRM_TUTORIAL,
        MENU_OPTIONS,
        MENU_OPTIONS_GAME,
        MENU_OPTIONS_INPUT,
        MENU_OPTIONS_AUDIO,
        MENU_OPTIONS_SAVE,
        MENU_OPTIONS_CANCEL,
        MENU_GAME_RESTART,
        MENU_GAME_RETURN_MAIN,
        MENU_GAME_RETURN_CONFIRM,
        MENU_GAME_QUIT
    };
    // enum Surface {
    //     DO_NOTHING,
    //     NEW_GAME,
    //     CONFIRM_TUTORIAL,
    //     MOVE_UP,
    //     MOVE_RIGHT,
    //     MOVE_DOWN,
    //     MOVE_LEFT,
    //     LOAD_GAME,
    //     RESTART,
    //     SAVE_GAME,
    //     OPTIONS,
    //     RESUME_GAME,
    //     MAIN_MENU,
    //     CONFIRM_RETURN,
    //     CANCEL_RETURN,
    //     CONFIRM_OPTIONS,
    //     CANCEL_OPTIONS,
    //     CHANGE_OPTIONS_GAME,
    //     CHANGE_OPTIONS_AUDIO,
    //     CHANGE_OPTIONS_INPUTS,
    //     QUIT_APP,
    // };
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
        TERTIARY,
        TERTIARY_DOWN,
        TERTIARY_UP,
        MOVE_UP,
        MOVE_RIGHT,
        MOVE_DOWN,
        MOVE_LEFT,
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

    // enum Surface {
    //     NO_EVENT,
    //     SHOW_TUTORIAL,
    //     CONFIRM_TUTORIAL,
    //     SHOW_RETURN_MAIN_MENU_CONFIRMATION,
    //     SHOW_OPTIONS,
    // };
}

namespace State {
    enum App {
        LOAD,
        RUN,
        HOLD,
        QUIT
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
        WIN,
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
    Vector2 mouseWheel;
};

#define DEFAULT_INPUT InputEvent({ .id = Event::Input::IDLE, .position = Vector2({}) })

struct GameState {
    int score;
    State::Game state;
    TimerId totalTimeId;
};

struct WorldState {
    bool reachedGoal;
    bool failedGoal;
};

class Layer {
public:
    virtual ~Layer() = default;
    virtual void resize(int width, int height) {}
};
