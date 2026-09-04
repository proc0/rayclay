#pragma once

#include "brick.h"
#include "type.hpp"

class Menu {
    Brick_ElementId bid_new;
    Brick_ElementId bid_quit;
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

	void transition(State::Screen);
	void unload();
};
