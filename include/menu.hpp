#pragma once

#include "brick.h"
#include "type.hpp"

// #include "raylib.h"

class Menu {
    // TODO: move this into a Display class
    // along with building the components
    // for the HUD
    // Texture2D textureArrowUp;
    // Texture2D textureArrowRight;
    // Texture2D textureArrowDown;
    // Texture2D textureArrowLeft;

    // Brick_ElementId bid_hudArrowUp;
    
    Brick_ComponentId bid_new;
    Brick_ComponentId bid_options;
    Brick_ComponentId bid_quit;
    
    Brick_ComponentId bid_options_save;
    Brick_ComponentId bid_options_cancel;
    Brick_ComponentId bid_resume;

    Brick_ComponentId bid_optionGame;
    Brick_ComponentId bid_optionInput;
    Brick_ComponentId bid_optionAudio;
    Brick_ComponentId bid_optionTabs;

    Brick_ElementId sid_string1;

    Brick_ContainerId cid_scrollBox;
    Brick_ContainerId cid_scrollBox2;

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
