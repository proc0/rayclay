#include "menu.hpp"

#include "brick.h"
// #include "index.h"
#include "text.hpp"

// #include "raylib.h"

void Menu::load() {
    bid_new = Brick_CreateButton("New Game");
    bid_options = Brick_CreateButton("Options");
    bid_quit = Brick_CreateButton("Quit");

    bid_resume = Brick_CreateButton("Resume");
    bid_options_save = Brick_CreateButton("Save");
    bid_options_cancel = Brick_CreateButton("Cancel");

	bid_optionGame = Brick_CreateToggleButton("Game");
    bid_optionInput = Brick_CreateToggleButton("Input");
    bid_optionAudio = Brick_CreateToggleButton("Audio");
    Brick_ElementId tabs[3] = { bid_optionGame, bid_optionInput, bid_optionAudio };
    bid_optionTabs = Brick_CreateButtonGroup(tabs, 3);

    // TODO: move this into Display class
    // textureArrowUp = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_UP));
    // textureArrowRight = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_RIGHT));
    // textureArrowDown = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_DOWN));
    // textureArrowLeft = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_LEFT));

    // bid_hudArrowUp = Brick_CreateImageButton(static_cast<float>(textureArrowUp.width), static_cast<float>(textureArrowUp.height), &textureArrowUp);

    cid_scrollBox = Brick_CreateScrollBox();
    cid_scrollBox2 = Brick_CreateScrollBox();
}

Action::Interface Menu::update() {
    Action::Interface action = Action::Interface::NOTHING;

    if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_new)) {
        action = Action::Interface::MENU_GAME_NEW;
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_resume)) {
        action = Action::Interface::MENU_GAME_RESUME;
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_quit)) {
        action = Action::Interface::MENU_GAME_QUIT;
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_options)) {
        layout = &Menu::layoutOptions;
    } else if (Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_options_save) || Brick_IsEventTriggeredById(BRICK_EVENT_TYPE_PRESS, bid_options_cancel)) {
        layout = &Menu::layoutMain;
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
            Brick_BeginHorizontalStack();
                Brick_BeginScrollBox(cid_scrollBox);
                    Brick_InlineText(TEXT_TUTORIAL_3);
                Brick_EndScrollBox();
                Brick_BeginScrollBox(cid_scrollBox2);
                    Brick_InlineText(TEXT_TUTORIAL_2);
                    Brick_InlineText(TEXT_TUTORIAL_1);
                Brick_EndScrollBox();
            Brick_EndHorizontalStack();
        } else if(Brick_IsButtonToggled(bid_optionAudio)) {
            Brick_BeginScrollBox(cid_scrollBox2);
                Brick_InlineText(TEXT_TUTORIAL_2);
                Brick_InlineText(TEXT_TUTORIAL_1);
            Brick_EndScrollBox();
        }

        Brick_BeginHorizontalStack();
            Brick_LayoutButton(bid_options_save);
            Brick_LayoutButton(bid_options_cancel);
        Brick_EndHorizontalStack();
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
    // UnloadTexture(textureArrowUp);
    // UnloadTexture(textureArrowRight);
    // UnloadTexture(textureArrowDown);
    // UnloadTexture(textureArrowLeft);
}