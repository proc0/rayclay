#include "interface.hpp"
#include "type.hpp"

#define BRICK_IMPLEMENTATION
#include "brick.h"
#include "index.h"

#include "raylib.h"

#include <cmath>
#include <cstring>

static Shader overlayShader;
static int overlayColorLocation;

static char *temp_render_buffer = NULL;
static int temp_render_buffer_len = 0;

Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    // Measure string size for Font
    Clay_Dimensions textSize = { 0 };

    float maxTextWidth = 0.0f;
    float lineTextWidth = 0;
    int maxLineCharCount = 0;
    int lineCharCount = 0;

    float textHeight = config->fontSize;
    Font* fonts = (Font*)userData;
    Font fontToUse = fonts[config->fontId];
    // Font failed to load, likely the fonts are in the wrong place relative to the execution dir.
    // RayLib ships with a default font, so we can continue with that built in one. 
    if (!fontToUse.glyphs) {
        fontToUse = GetFontDefault();
    }

    float scaleFactor = config->fontSize/(float)fontToUse.baseSize;

    for (int i = 0; i < text.length; ++i, lineCharCount++)
    {
        if (text.chars[i] == '\n') {
            maxTextWidth = fmax(maxTextWidth, lineTextWidth);
            maxLineCharCount = CLAY__MAX(maxLineCharCount, lineCharCount);
            lineTextWidth = 0;
            lineCharCount = 0;
            continue;
        }
        int index = text.chars[i] - 32;
        if (fontToUse.glyphs[index].advanceX != 0) lineTextWidth += fontToUse.glyphs[index].advanceX;
        else lineTextWidth += (fontToUse.recs[index].width + fontToUse.glyphs[index].offsetX);
    }

    maxTextWidth = fmax(maxTextWidth, lineTextWidth);
    maxLineCharCount = CLAY__MAX(maxLineCharCount, lineCharCount);

    textSize.width = maxTextWidth * scaleFactor + (lineCharCount * config->letterSpacing);
    textSize.height = textHeight;

    return textSize;
}

void Interface::loadOverlay() {
#ifdef __EMSCRIPTEN__
    // GLSL ES 3.0 shader for WebGL 2.0 used by Emscripten for Web
    const char* overlayShaderCode = 
        "#version 300 es\n"
        "precision mediump float;\n"
        "\n"
        "in vec2 fragTexCoord;\n"
        "in vec4 fragColor;\n"
        "\n"
        "uniform sampler2D texture0;\n"
        "uniform vec4 overlayColor;\n"
        "\n"
        "out vec4 finalColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec4 texelColor = texture(texture0, fragTexCoord) * fragColor;\n"
        "    vec3 blendedRGB = mix(texelColor.rgb, overlayColor.rgb, overlayColor.a);\n"
        "    finalColor = vec4(blendedRGB, texelColor.a);\n"
        "}";
#else
    // GLSL 3.3 for OpenGL 3 used for Desktop 
    const char* overlayShaderCode = 
        "#version 330\n"
        "\n"
        "in vec2 fragTexCoord;\n"
        "in vec4 fragColor;\n"
        "\n"
        "uniform sampler2D texture0;\n"
        "uniform vec4 overlayColor;\n"
        "\n"
        "out vec4 finalColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec4 texelColor = texture(texture0, fragTexCoord) * fragColor;\n"
        "    vec3 blendedRGB = mix(texelColor.rgb, overlayColor.rgb, overlayColor.a);\n"
        "    finalColor = vec4(blendedRGB, texelColor.a);\n"
        "}";
#endif
    overlayShader = LoadShaderFromMemory(0, overlayShaderCode);
    overlayColorLocation = GetShaderLocation(overlayShader, "overlayColor");
}

void Interface::load() {
    fonts[0] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_MEDIUM), 48, 0, 400);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    fonts[1] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_REGULAR), 32, 0, 400);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);

    Brick_Initialize(window.widthf, window.heightf, Raylib_MeasureText, fonts);

    loadOverlay();

    menu.load();
}

Action::Interface Interface::update(const InputEvent& inputEvent) {
    Brick_EventArray eventArray = Brick_UpdateEvents({ 
        .x = inputEvent.position.x, 
        .y = inputEvent.position.y, 
        .pressed = inputEvent.id == Event::Input::PRIMARY || inputEvent.id == Event::Input::PRIMARY_DOWN
    });

    Action::Interface action = Action::Interface::NOTHING;
    
    for (int i=0; i<eventArray.length; i++) {
        Brick_Event* event = Brick_EventArray_Get(&eventArray, i);

        switch(event->eventType) {
        case BRICK_EVENT_PRESS:
            TraceLog(LOG_INFO, "CLICK FROM APP HOORAY");
        break;
        case BRICK_EVENT_PRESSING:
            // TraceLog(LOG_INFO, "CLICKING...");
        break;
        case BRICK_EVENT_RELEASE:
            // TraceLog(LOG_INFO, "RELEASE");
        break;
        case BRICK_EVENT_HOVER:
            // TraceLog(LOG_INFO, "JUST HOVERED");
            // SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        break;
        case BRICK_EVENT_HOVERING:
            // TraceLog(LOG_INFO, "HOVERING...");
        break;
        case BRICK_EVENT_CLEAR:
            // TraceLog(LOG_INFO, "JUST CLEARED");
            // SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        break;
        default: break;
        }
    }

    action = menu.update();

    if (Brick_IsEventTriggered(BRICK_EVENT_HOVER)) {
        // TraceLog(LOG_INFO, "RAYLIB: JUST HOVERED");
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (Brick_IsEventTriggered(BRICK_EVENT_CLEAR)) {
        // TraceLog(LOG_INFO, "RAYLIB: JUST CLEARED");
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    return action;
}

Clay_RenderCommandArray Interface::layout(const InputEvent& inputEvent) {
    Brick_BeginLayout();
        (menu.*menu.layout)();
    return Brick_EndLayout(GetFrameTime());
}

void Interface::render(Clay_RenderCommandArray& renderCommands) const {
    for (int j = 0; j < renderCommands.length; ++j) {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = {renderCommand->boundingBox.x, renderCommand->boundingBox.y, renderCommand->boundingBox.width, renderCommand->boundingBox.height};
        // TraceLog(LOG_INFO, "RENDER COMM %d", renderCommand->commandType);
        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData *textData = &renderCommand->renderData.text;
                Font fontToUse = fonts[textData->fontId];
    
                int strlen = textData->stringContents.length + 1;
    
                if(strlen > temp_render_buffer_len) {
                    // Grow the temp buffer if we need a larger string
                    if(temp_render_buffer) free(temp_render_buffer);
                    temp_render_buffer = (char *) malloc(strlen);
                    temp_render_buffer_len = strlen;
                }
    
                // Raylib uses standard C strings so isn't compatible with cheap slices, we need to clone the string to append null terminator
                memcpy(temp_render_buffer, textData->stringContents.chars, textData->stringContents.length);
                temp_render_buffer[textData->stringContents.length] = '\0';
                DrawTextEx(fontToUse, temp_render_buffer, Vector2({boundingBox.x, boundingBox.y}), static_cast<float>(textData->fontSize), static_cast<float>(textData->letterSpacing), CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor));
                // DrawText(temp_render_buffer, boundingBox.x, boundingBox.y, static_cast<float>(textData->fontSize), CLAY_COLOR_TO_RAYLIB_COLOR(textData->textColor));
    
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Texture2D texture = *static_cast<Texture2D*>(renderCommand->renderData.image.imageData);
                DrawTexturePro(
                    texture,
                    Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                    Rectangle({boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }),
                    Vector2({}), 0, WHITE);
                // NOTE: this is applying a tint based on image.backgroundColor take from the example rendered, 
                // but the current Clay version does not have this property on image. Keeping for backup.
                // Color tintColor = WHITE;
                // Clay_Color imageBgColor = renderCommand->renderData.image.backgroundColor;
                // if (imageBgColor.r > 0 || imageBgColor.g > 0 || imageBgColor.b > 0 || imageBgColor.a > 0) {
                //     tintColor = CLAY_COLOR_TO_RAYLIB_COLOR(imageBgColor);
                // }

                // NOTE: .userData + .image = scroll image feature! If userData and imageData are both set, scroll the image.
                // If .userData is needed for something that also has an .image, modify these conditional safety checks.
                // if (ScrollState* scrollState = static_cast<ScrollState*>(renderCommand->userData)) {
                //     // match the parent ID or another "proxy" container ID with the current rendering ID.
                //     // which allows its image to be controlled by the scroll vertical movement. This prevents accidental scrolling.
                //     if (scrollState->parentId.id == renderCommand->id || scrollState->proxyId.id == renderCommand->id) {
                //         // calculate the background image offset based on the vertical scroll position
                //         // as the scroll position changes, render two backgrounds one after another
                //         float imageOffset = scrollState->scrollY;
                //         // WARNING: scrollY can be negative, wrap around
                //         if (scrollState->scrollY + boundingBox.height <= 0) {
                //             imageOffset = static_cast<float>(static_cast<int>(roundf(scrollState->scrollY)) % static_cast<int>(roundf(boundingBox.height)));
                //         }
                //         // NOTE: Scissor mode is used here to trim the extra image after doubling it for scrolling. 
                //         // This prevents any sibling elements from rendering properly. The right approach would open the Scissor tag
                //         // and then close it after all the siblings have done rendering...
                //         BeginScissorMode(static_cast<int>(roundf(boundingBox.x)), static_cast<int>(roundf(boundingBox.y)), static_cast<int>(roundf(boundingBox.width)), static_cast<int>(roundf(boundingBox.height)));
                //         DrawTexturePro(
                //             texture,
                //             Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                //             Rectangle({boundingBox.x, boundingBox.y+imageOffset, boundingBox.width, boundingBox.height }),
                //             Vector2({}), 0, WHITE);
                //         DrawTexturePro(
                //             texture,
                //             Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                //             Rectangle({boundingBox.x, boundingBox.y+boundingBox.height+imageOffset, boundingBox.width, boundingBox.height }),
                //             Vector2({}), 0, WHITE);
                //         EndScissorMode();
                //     }
                // } else {
                //     DrawTexturePro(
                //         texture,
                //         Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                //         Rectangle({boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }),
                //         Vector2({}), 0, WHITE);
                // }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {

                BeginScissorMode(static_cast<int>(roundf(boundingBox.x)), static_cast<int>(roundf(boundingBox.y)), static_cast<int>(roundf(boundingBox.width)), static_cast<int>(roundf(boundingBox.height)));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START: {
                // NOTE: converting to Raylib unsigned char 0-255 first, then dividing by 255 
                // (or multiplying by 1/255) because shader requires rgba values between 0.0f-1.0f
                Color color = CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->renderData.overlayColor.color);
                float colorFloat[4] = {
                    static_cast<float>(color.r)*INV255,
                    static_cast<float>(color.g)*INV255,
                    static_cast<float>(color.b)*INV255,
                    static_cast<float>(color.a)*INV255,
                };
                SetShaderValue(overlayShader, overlayColorLocation, colorFloat, SHADER_UNIFORM_VEC4);
                // NOTE: in the Raylib Renderer example of Clay, this was behind a bool flag
                // that was turn on to call EndShaderMode in CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END 
                BeginShaderMode(overlayShader);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END: {
                // NOTE: in the Raylib Renderer example of Clay, this was behind a bool flag
                // that if on, calls EndShaderMode and then turns off the flag
                EndShaderMode();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleRenderData *config = &renderCommand->renderData.rectangle;
                if (config->cornerRadius.topLeft > 0) {
                    float radius = (config->cornerRadius.topLeft * 2) / (float)((boundingBox.width > boundingBox.height) ? boundingBox.height : boundingBox.width);
                    DrawRectangleRounded(Rectangle({ boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }), radius, 8, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
                } else {
                    DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, CLAY_COLOR_TO_RAYLIB_COLOR(config->backgroundColor));
                }
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData *config = &renderCommand->renderData.border;
                // Left border
                if (config->width.left > 0) {
                    DrawRectangleV(Vector2({ boundingBox.x, boundingBox.y + config->cornerRadius.topLeft }), Vector2({ static_cast<float>(config->width.left), boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft }), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Right border
                if (config->width.right > 0) {
                    DrawRectangleV(Vector2({ boundingBox.x + boundingBox.width - config->width.right, boundingBox.y + config->cornerRadius.topRight }), Vector2({ static_cast<float>(config->width.right), boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight }), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Top border
                if (config->width.top > 0) {
                    DrawRectangleV(Vector2({ boundingBox.x + config->cornerRadius.topLeft, boundingBox.y }), Vector2({ boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight, static_cast<float>(config->width.top) }), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                // Bottom border
                if (config->width.bottom > 0) {
                    DrawRectangleV(Vector2({ boundingBox.x + config->cornerRadius.bottomLeft, boundingBox.y + boundingBox.height - config->width.bottom }), Vector2({ boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight, static_cast<float>(config->width.bottom) }), CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.topLeft > 0) {
                    DrawRing(Vector2({ roundf(boundingBox.x + config->cornerRadius.topLeft), roundf(boundingBox.y + config->cornerRadius.topLeft) }), roundf(config->cornerRadius.topLeft - config->width.top), config->cornerRadius.topLeft, 180, 270, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.topRight > 0) {
                    DrawRing(Vector2({ roundf(boundingBox.x + boundingBox.width - config->cornerRadius.topRight), roundf(boundingBox.y + config->cornerRadius.topRight) }), roundf(config->cornerRadius.topRight - config->width.top), config->cornerRadius.topRight, 270, 360, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.bottomLeft > 0) {
                    DrawRing(Vector2({ roundf(boundingBox.x + config->cornerRadius.bottomLeft), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft) }), roundf(config->cornerRadius.bottomLeft - config->width.bottom), config->cornerRadius.bottomLeft, 90, 180, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                if (config->cornerRadius.bottomRight > 0) {
                    DrawRing(Vector2({ roundf(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight) }), roundf(config->cornerRadius.bottomRight - config->width.bottom), config->cornerRadius.bottomRight, 0.1, 90, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
                }
                break;
            }
            // case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
            //     Clay_CustomRenderData *config = &renderCommand->renderData.custom;
            //     CustomLayoutElement *customElement = (CustomLayoutElement *)config->customData;
            //     if (!customElement) continue;
            //     switch (customElement->type) {
            //         case CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL: {
            //             Clay_BoundingBox rootBox = renderCommands.internalArray[0].boundingBox;
            //             float scaleValue = CLAY__MIN(CLAY__MIN(1, 768 / rootBox.height) * CLAY__MAX(1, rootBox.width / 1024), 1.5f);
            //             Ray positionRay = getScreenToWorldPointWithZDistance(Vector2({ renderCommand->boundingBox.x + renderCommand->boundingBox.width / 2, renderCommand->boundingBox.y + (renderCommand->boundingBox.height / 2) + 20 }), Raylib_camera, static_cast<int>(roundf(rootBox.width)), static_cast<int>(roundf(rootBox.height)), 140);
            //             BeginMode3D(Raylib_camera);
            //                 DrawModel(customElement->customData.model.model, positionRay.position, customElement->customData.model.scale * scaleValue, WHITE);        // Draw 3d model with texture
            //             EndMode3D();
            //             break;
            //         }
            //         default: break;
            //     }
            //     break;
            // }
            default: {
                TraceLog(LOG_ERROR, "Error: unhandled render command.");
                exit(1);
            }
        }
    }
}

void Interface::transition(State::App state, State::Screen screen) {
	menu.transition(state, screen);
}

void Interface::unload() {
    menu.unload();

    if(temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer_len = 0;

    Brick_Destroy();

    for (auto& font : fonts) {
        UnloadFont(font);
    }
}
