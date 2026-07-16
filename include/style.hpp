#pragma once


#define CLAY_WHITE Clay_Color({ 255.0f, 255.0f, 255.0f, 255.0f })
#define CLAY_BLACK Clay_Color({ 0.0f, 0.0f, 0.0f, 255.0f })

#define SURFACE_COLOR_MENU_BG Clay_Color({ 0, 0, 0, 150 })
#define SURFACE_COLOR_TEXT_FG Clay_Color({ 200, 200, 200, 255 })
#define SURFACE_COLOR_TEXT_FG_HL Clay_Color({ 255, 255, 255, 255 })

#define WIDGET_COLOR_BUTTON_BG Clay_Color({ 0, 0, 0, 100 })
#define WIDGET_COLOR_BUTTON_BG_HL Clay_Color({ 0, 0, 0, 130 })
#define WIDGET_COLOR_BORDER Clay_Color({ 220, 220, 220, 255 })

#define WIDGET_COLOR_SCROLLBAR Clay_Color({100, 100, 140, 150})
#define WIDGET_COLOR_SCROLLBAR_HL Clay_Color({120, 120, 160, 150})

// typedef struct Clay_TextElementConfig {
//     // A pointer that will be transparently passed through to the resulting render command.
//     void *userData;
//     // The RGBA color of the font to render, conventionally specified as 0-255.
//     Clay_Color textColor;
//     // An integer transparently passed to Clay_MeasureText to identify the font to use.
//     // The debug view will pass fontId = 0 for its internal text.
//     uint16_t fontId;
//     // Controls the size of the font. Handled by the function provided to Clay_MeasureText.
//     uint16_t fontSize;
//     // Controls extra horizontal spacing between characters. Handled by the function provided to Clay_MeasureText.
//     uint16_t letterSpacing;
//     // Controls additional vertical space between wrapped lines of text.
//     uint16_t lineHeight;
//     // Controls how text "wraps", that is how it is broken into multiple lines when there is insufficient horizontal space.
//     // CLAY_TEXT_WRAP_WORDS (default) breaks on whitespace characters.
//     // CLAY_TEXT_WRAP_NEWLINES doesn't break on space characters, only on newlines.
//     // CLAY_TEXT_WRAP_NONE disables wrapping entirely.
//     Clay_TextElementConfigWrapMode wrapMode;
//     // Controls how wrapped lines of text are horizontally aligned within the outer text bounding box.
//     // CLAY_TEXT_ALIGN_LEFT (default) - Horizontally aligns wrapped lines of text to the left hand side of their bounding box.
//     // CLAY_TEXT_ALIGN_CENTER - Horizontally aligns wrapped lines of text to the center of their bounding box.
//     // CLAY_TEXT_ALIGN_RIGHT - Horizontally aligns wrapped lines of text to the right hand side of their bounding box.
//     Clay_TextAlignment textAlignment;
// } Clay_TextElementConfig;

#define STYLE_TEXT_DEFAULT CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_TEXT_FG, .fontSize = 24 })
#define STYLE_TEXT_TITLE CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_TEXT_FG, .fontSize = 32, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
#define STYLE_TEXT_DISPLAY CLAY_TEXT_CONFIG({ .textColor = CLAY_WHITE, .fontSize = 48, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
