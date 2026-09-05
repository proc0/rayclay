#pragma once

#include "brick.h"
#include "type.hpp"

#include "raylib.h"

class Menu {
    // TODO: move this into a Display class
    // along with building the components
    // for the HUD
    Texture2D textureArrowUp;
    Texture2D textureArrowRight;
    Texture2D textureArrowDown;
    Texture2D textureArrowLeft;

    Brick_ElementId bid_hudArrowUp;
    
    Brick_ElementId bid_new;
    Brick_ElementId bid_options;
    Brick_ElementId bid_quit;
    
    Brick_ElementId bid_options_save;
    Brick_ElementId bid_options_cancel;
    Brick_ElementId bid_resume;

    Brick_ElementId bid_optionGame;
    Brick_ElementId bid_optionInput;
    Brick_ElementId bid_optionAudio;
    Brick_ElementId bid_optionTabs;

public:
	Menu(){}
	~Menu() = default;

	void (Menu::*layout)() = &Menu::layoutUnit;

	void load();

	void layoutUnit() {};
	void layoutMain();
	void layoutOptions();
	void layoutGame();
	Action::Interface update();
	void render() const;

	void transition(State::App, State::Screen);
	void unload();
};
