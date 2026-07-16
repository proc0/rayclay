#pragma once


#define CLAY_WHITE Clay_Color({ 255.0f, 255.0f, 255.0f, 255.0f })
#define CLAY_BLACK Clay_Color({ 0.0f, 0.0f, 0.0f, 255.0f })
#define SURFACE_BUTTON_COLOR_BG Clay_Color({ 0, 0, 0, 100 })
#define SURFACE_BUTTON_COLOR_BG_HL Clay_Color({ 0, 0, 0, 130 })
#define SURFACE_BUTTON_COLOR_FG Clay_Color({ 200, 200, 200, 255 })
#define SURFACE_BUTTON_COLOR_FG_HL Clay_Color({ 255, 255, 255, 255 })
#define SURFACE_MENU_COLOR_BG Clay_Color({ 0, 0, 0, 150 })

#define STYLE_TEXT_DEFAULT CLAY_TEXT_CONFIG({ .textColor = SURFACE_BUTTON_COLOR_FG, .fontSize = 24 })
