#pragma once

#ifndef RAYLIB_CLAY_RENDERER
#define RAYLIB_CLAY_RENDERER

#include "clay.h"

#include "raylib.h"

#define INV255 (1.0f/255.0f)
#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) Clay_Vector2({ .x = vector.x, .y = vector.y })
#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) Rectangle({ .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height })
// NOTE: if color conversion is off, check Clay color representation type, it might need to be rounded. Clay uses 0.0f-255.0f, Raylib is unsighed char.
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) Color({ .r = static_cast<unsigned char>(roundf(color.r)), .g = static_cast<unsigned char>(roundf(color.g)), .b = static_cast<unsigned char>(roundf(color.b)), .a = static_cast<unsigned char>(roundf(color.a)) })
#define RAYLIB_COLOR_TO_CLAY_COLOR(color) Clay_Color({ static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b), static_cast<float>(color.a) })

static Shader overlayShader;
static int colorLoc;
static int overlayColorLocation;

static char *temp_render_buffer = NULL;
static int temp_render_buffer_len = 0;

void LoadOverlay();
void RenderRaylib(Clay_RenderCommandArray& renderCommands);
void Clay_Raylib_Close();

#endif
