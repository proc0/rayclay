#pragma once

#include "config.hpp"
#include "type.hpp"
#include "window.hpp"

#include "raylib.h"

class Logo : public Layer {
    const char* logoName = "proc0";
    int logoFontSize = 108;

    const char* rlLogoName = "raylib";
    const int rlLogoFontSize = 40;
    const float rlLogoSize = RAYLIB_LOGO_SIZE;
    const int rlLogoBorder = 16;
    const float rlLogoInnerSize = rlLogoSize - 2.0f*rlLogoBorder;
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
