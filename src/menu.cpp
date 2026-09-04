#include "menu.hpp"

#include "brick.h"

void Menu::load() {
    bid_new = Brick_CreateButton("New Game");
    bid_options = Brick_CreateButton("Options");
    bid_quit = Brick_CreateButton("Quit");

    // bid_pause = Brick_CreateButton("Pause");
    bid_resume = Brick_CreateButton("Resume");

	bid_optionGame = Brick_CreateToggleButton("Game");
    bid_optionInput = Brick_CreateToggleButton("Input");
    bid_optionAudio = Brick_CreateToggleButton("Audio");
    Brick_ElementId tabs[3] = { bid_optionGame, bid_optionInput, bid_optionAudio };
    bid_optionTabs = Brick_CreateButtonGroup(tabs, 3);
}

Action::Interface Menu::update() {
    Action::Interface action = Action::Interface::NOTHING;

    if (Brick_IsEventTriggeredById(BRICK_EVENT_PRESS, bid_new)) {
        action = Action::Interface::MENU_GAME_NEW;
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_PRESS, bid_resume)) {
        action = Action::Interface::MENU_GAME_RESUME;
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_PRESS, bid_quit)) {
        action = Action::Interface::MENU_GAME_QUIT;
    }

    return action;
}

void Menu::layoutMain() {
    Brick_BeginFloatingPanel();
        Brick_LayoutButton(bid_new);
        Brick_LayoutButton(bid_options);
        Brick_LayoutButton(bid_quit);
    Brick_EndFloatingPanel();
}

void Menu::layoutGame() {
    Brick_BeginFloatingPanel();
        Brick_LayoutButton(bid_resume);
    Brick_EndFloatingPanel();
}

void Menu::layoutOptions() {
    Brick_BeginFloatingPanel();
        Brick_BeginHorizontalStack();
            Brick_LayoutButtonGroup(bid_optionTabs);
        Brick_EndHorizontalStack();

        if(Brick_IsButtonToggled(bid_optionGame)) {
            Brick_BeginPanel();
                Brick_InlineText("TAB 1");
            Brick_EndPanel();
        } else if(Brick_IsButtonToggled(bid_optionInput)) {
            Brick_BeginPanel();
                Brick_InlineText("This is the second tab.");
            Brick_EndPanel();
        } else if(Brick_IsButtonToggled(bid_optionAudio)) {
            Brick_BeginPanel();
                Brick_InlineText("3rd TAB!!");
            Brick_EndPanel();
        }
    Brick_EndFloatingPanel();
}

void Menu::render() const {
	
}

void Menu::transition(State::App state, State::Screen screen) {
    switch(screen) {
    case State::Screen::MAIN:
        layout = &Menu::layoutMain;
    break;
    case State::Screen::GAME:
        layout = state == State::App::HOLD ? &Menu::layoutGame : &Menu::layoutUnit;
    break;
    default: 
        layout = &Menu::layoutUnit;
    }
}

void Menu::unload() {
	
}