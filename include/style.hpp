#pragma once

#define CLAY_BLANK Clay_Color({ 0, 0, 0, 0 })
#define CLAY_WHITE Clay_Color({ 255.0f, 255.0f, 255.0f, 255.0f })
#define CLAY_BLACK Clay_Color({ 0, 0, 0, 255.0f })

#define SURFACE_COLOR_BG Clay_Color({ 140, 140, 140, 255 })
#define SURFACE_COLOR_FG Clay_Color({ 200, 200, 200, 255 })
#define SURFACE_COLOR_HL Clay_Color({ 235, 235, 235, 255 })
#define SURFACE_COLOR_SECONDARY Clay_Color({ 80, 80, 80, 255 })

#define SURFACE_COLOR_MENU_BG Clay_Color({ 0, 0, 0, 180 })
#define SURFACE_COLOR_ACCENT_BG Clay_Color({ 140, 140, 140, 255 })
#define SURFACE_COLOR_ACCENT_BORDER Clay_Color({ 80, 80, 80, 255 })
#define SURFACE_COLOR_ACCENT_RED Clay_Color({ 230, 40, 45, 255 })
#define SURFACE_COLOR_ACCENT_GOLD Clay_Color({ 216, 238, 10, 255 })

#define WIDGET_COLOR_BUTTON_BG Clay_Color({ 80, 80, 80, 255 })
#define WIDGET_COLOR_BUTTON_BG_HL Clay_Color({ 90, 90, 90, 255 })
#define WIDGET_COLOR_BORDER Clay_Color({ 200, 200, 200, 255 })

#define WIDGET_COLOR_SCROLLBAR Clay_Color({80, 80, 80, 255})
#define WIDGET_COLOR_SCROLLBAR_HL Clay_Color({140, 140, 140, 255})

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

#define STYLE_TEXT_DEFAULT CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_FG, .fontId = 1, .fontSize = 24 })
#define STYLE_TEXT_CENTERED CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_FG, .fontId = 1, .fontSize = 24, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
#define STYLE_TEXT_DISABLED CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_BG, .fontId = 1, .fontSize = 24 })
#define STYLE_TEXT_TITLE CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_BG, .fontSize = 32, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
#define STYLE_TEXT_HIGHLIGHT CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_HL, .fontSize = 32, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
#define STYLE_TEXT_DISPLAY CLAY_TEXT_CONFIG({ .textColor = CLAY_WHITE, .fontSize = 48, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
#define STYLE_TEXT_BANNER CLAY_TEXT_CONFIG({ .textColor = CLAY_WHITE, .fontSize = 42, .textAlignment = CLAY_TEXT_ALIGN_CENTER })
#define STYLE_TEXT_WIN CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_ACCENT_GOLD, .fontId = 2, .fontSize = 160 })
#define STYLE_TEXT_LOSE CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_ACCENT_RED, .fontId = 2, .fontSize = 160 })

// typedef struct Clay_ElementDeclaration {
//     // Controls various settings that affect the size and position of an element, as well as the sizes and positions of any child elements.
//     Clay_LayoutConfig layout;
//     // Controls the background color of the resulting element.
//     // By convention specified as 0-255, but interpretation is up to the renderer.
//     // If no other config is specified, .backgroundColor will generate a RECTANGLE render command, otherwise it will be passed as a property to IMAGE or CUSTOM render commands.
//     Clay_Color backgroundColor;
//     // Perform an image editing style "Color Overlay" on this element and all its children, equivalent to
//     // glsl mix(elementColor, overlayColor.rgb, overlayColor.a)
//     Clay_Color overlayColor;
//     // Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
//     Clay_CornerRadius cornerRadius;
//     // Controls settings related to aspect ratio scaling.
//     Clay_AspectRatioElementConfig aspectRatio;
//     // Controls settings related to image elements.
//     Clay_ImageElementConfig image;
//     // Controls whether and how an element "floats", which means it layers over the top of other elements in z order, and doesn't affect the position and size of siblings or parent elements.
//     // Note: in order to activate floating, .floating.attachTo must be set to something other than the default value.
//     Clay_FloatingElementConfig floating;
//     // Used to create CUSTOM render commands, usually to render element types not supported by Clay.
//     Clay_CustomElementConfig custom;
//     // Controls whether an element should clip its contents, as well as providing child x,y offset configuration for scrolling.
//     Clay_ClipElementConfig clip;
//     // Controls settings related to element borders, and will generate BORDER render commands.
//     Clay_BorderElementConfig border;
//     Clay_TransitionElementConfig transition;
//     // A pointer that will be transparently passed through to resulting render commands.
//     void *userData;
// } Clay_ElementDeclaration;
