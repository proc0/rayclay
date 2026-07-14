#include "logo.hpp"

#include "raylib.h"

void Logo::load() {
    float rlLogoX = window.halfWidthf*1.4f - rlLogoSize*0.5f;
    float rlLogoY = window.halfHeightf - rlLogoSize*0.5f;
    drawRaylibLogo({ rlLogoX, rlLogoY });
}

void Logo::drawRaylibLogo(Vector2 logoPosition) {
    float rlLogoInnerX = logoPosition.x + rlLogoBorder;
    float rlLogoInnerY = logoPosition.y + rlLogoBorder;

    int rlLogoTextSize = MeasureText(rlLogoName, rlLogoFontSize);
    float rlLogoTextX = logoPosition.x + rlLogoSize - rlLogoTextSize - 2.0f*rlLogoBorder;
    float rlLogoTextY = logoPosition.y + rlLogoSize - rlLogoFontSize - 1.5f*rlLogoBorder;

    rlLogoOuterRec = { logoPosition.x, logoPosition.y, rlLogoSize, rlLogoSize };
    rlLogoInnerRec = { rlLogoInnerX, rlLogoInnerY, rlLogoInnerSize, rlLogoInnerSize };
    rlLogoTextPos = { rlLogoTextX, rlLogoTextY };
}

void Logo::renderRaylibLogo() const {
    DrawRectangleRec(rlLogoOuterRec, BLACK);
    DrawRectangleRec(rlLogoInnerRec, RAYWHITE);
    DrawText(rlLogoName, rlLogoTextPos.x, rlLogoTextPos.y, rlLogoFontSize, BLACK);
}

void Logo::renderLogo() const {
    // float logoSize = MeasureText(logoName, logoFontSize);
    int logoX = window.halfWidthf*0.4f;
    int logoY = window.halfHeightf - logoFontSize*0.4f;

    DrawText(logoName, logoX, logoY, logoFontSize, BLACK);
}

void Logo::render() const {
    ClearBackground(RAYWHITE);
    renderRaylibLogo();
    renderLogo();
}

void Logo::resize(int width, int height) {
    load();
}

void Logo::unload() {

}
