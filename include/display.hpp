#pragma once

#include "brick.h"
#include "type.hpp"
#include "window.hpp"

#include "raylib.h"

class Display {
    Texture2D textureArrowUp;
    Texture2D textureArrowRight;
    Texture2D textureArrowDown;
    Texture2D textureArrowLeft;

    Brick_ElementId bid_arrowUp;
    Brick_ElementId bid_arrowRight;
    Brick_ElementId bid_arrowDown;
    Brick_ElementId bid_arrowLeft;

	const Window& window;

public:
	Display(const Window& window): window(window) {}
	~Display() = default;

	void (Display::*layout)() = &Display::layoutUnit;

	void load();

	void layoutUnit() {};
	void layoutGame();
	Action::Display update();
	void render() const;

	void transition(State::App, State::Screen);
	void unload();
};
