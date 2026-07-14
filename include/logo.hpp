#pragma once

#include "type.hpp"
#include "window.hpp"

#include "raylib.h"

class Logo : public Layer {
    const char* logoName = "proc0";
    int logoFontSize = 108;

    const char* rlLogoName = "raylib";
    const int rlLogoFontSize = 40;
    const float rlLogoSize = 200;
    const int rlLogoBorder = 16;
    const float rlLogoInnerSize = rlLogoSize - 2.0f*rlLogoBorder;
    Vector2 rlLogoDir = { static_cast<float>(GetRandomValue(-100, 100))/100.0f, static_cast<float>(GetRandomValue(-100, 100))/100.0f };
    Vector2 rlLogoPos = { static_cast<float>(GetRandomValue(0, SCREEN_WIDTH-rlLogoSize)), static_cast<float>(GetRandomValue(0, SCREEN_HEIGHT-rlLogoSize)) };
    Rectangle rlLogoOuterRec;
    Rectangle rlLogoInnerRec;
    Vector2 rlLogoTextPos;

	const Window& window;

public:
	Logo(const Window& window): window(window)  {};
	~Logo() = default;

	void load();

	void render() const;
	void renderRaylibLogo() const;
	void renderLogo() const;
	void drawRaylibLogo(Vector2 logoPosition);
	
	void unload();

	void resize(int width, int height) override;
};
