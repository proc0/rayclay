#pragma once

#include "brick.h"
#include "type.hpp"

class Menu {
    Brick_ElementId buttonId;
    Brick_ElementId buttonId2;
    Brick_ElementId buttonId3;
    Brick_ElementId buttonGroupId1;

public:
	Menu(){}
	~Menu() = default;

	void load();

	void layout();
	Action::Interface update();
	void render() const;

	void unload();
};
