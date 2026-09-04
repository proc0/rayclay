#pragma once

#include "brick.h"

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
	void update();
	void render() const;

	void unload();
};
