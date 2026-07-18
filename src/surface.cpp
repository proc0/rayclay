#define CLAY_IMPLEMENTATION
#include "surface.hpp"

#include "index.h"
#include "type.hpp"
#include "text.hpp"
#include "style.hpp"

// #include "clay.h"
#include "raylib.h"
#include "raymath.h"

// #include <stdint.h>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

static inline Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
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

void Surface::load(){
    // 1. Query minimum memory required for default element limits
    uint64_t memorySize = Clay_MinMemorySize();
    // 2. Allocate memory (malloc, stack, or custom allocator)
    void* memory = malloc(memorySize);
    // 3. Create arena [clay.h:2150-2158]
    arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    // 4. Initialize Clay [clay.h:2186-2188]
    Clay_Initialize(arena, Clay_Dimensions({ window.widthf, window.heightf }), Clay_ErrorHandler({ .errorHandlerFunction = handleError, .userData = this }));

    fonts[0] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_MEDIUM), 48, 0, 400);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    fonts[1] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_REGULAR), 32, 0, 400);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);
    fonts[2] = LoadFontEx(PATH_ASSET(URI_FONT_ROBOTO_CONDENSED), 184, 0, 400);
    SetTextureFilter(fonts[2].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    loadOverlay();

    // TODO: move this into Display class
    textureArrowUp = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_UP));
    textureArrowRight = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_RIGHT));
    textureArrowDown = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_DOWN));
    textureArrowLeft = LoadTexture(PATH_ASSET(URI_IMAGE_ARROW_LEFT));

    textureBlueTile = LoadTexture(PATH_ASSET(URI_IMAGE_BLUE_TILE));

    // INIT: scrollbar Ids, one for the container, one for the scrollbar, and one for a proxy
    // of the image that can be scrolled.
    // TODO: make the proxyId optional by overloading
    // widget.initScrollbar(scrollbarTutorialContainerId, scrollbarTutorialId, layoutTutorialId);
    // TODO: this would receive a ScrollStateId of some kind, and that would be used to open/close
    // scrollBox blacks, i.e.:
    // tutorialScrollId = widget.initScrollBox("TutorialScrollBox");
    widget.initScrollBox("TutorialScrollBox");
}

void Surface::loadOverlay() {
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

// Get a ray trace from the screen position (i.e mouse) within a specific section of the screen
Ray getScreenToWorldPointWithZDistance(Vector2 position, Camera camera, int screenWidth, int screenHeight, float zDistance)
{
    Ray ray = { 0 };

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*position.x)/(float)screenWidth - 1.0f;
    float y = 1.0f - (2.0f*position.y)/(float)screenHeight;
    float z = 1.0f;

    // Store values in a vector
    Vector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)screenWidth/(double)screenHeight), 0.01f, zDistance);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)screenWidth/(double)screenHeight;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    Vector3 nearPoint = Vector3Unproject(Vector3({ deviceCoords.x, deviceCoords.y, 0.0f }), matProj, matView);
    Vector3 farPoint = Vector3Unproject(Vector3({ deviceCoords.x, deviceCoords.y, 1.0f }), matProj, matView);

    // Calculate normalized direction vector
    Vector3 direction = Vector3Normalize(Vector3Subtract(farPoint, nearPoint));

    ray.position = farPoint;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// A MALLOC'd buffer, that we keep modifying inorder to save from so many Malloc and Free Calls.
// frees in unload method
static inline char *temp_render_buffer;
static inline int temp_render_buffer_len;

void Surface::renderRaylib(Clay_RenderCommandArray& renderCommands) const {
    for (int j = 0; j < renderCommands.length; ++j) {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = {renderCommand->boundingBox.x, renderCommand->boundingBox.y, renderCommand->boundingBox.width, renderCommand->boundingBox.height};
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
    
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Texture2D texture = *static_cast<Texture2D*>(renderCommand->renderData.image.imageData);
                // NOTE: this is applying a tint based on image.backgroundColor take from the example rendered, 
                // but the current Clay version does not have this property on image. Keeping for backup.
                // Color tintColor = WHITE;
                // Clay_Color imageBgColor = renderCommand->renderData.image.backgroundColor;
                // if (imageBgColor.r > 0 || imageBgColor.g > 0 || imageBgColor.b > 0 || imageBgColor.a > 0) {
                //     tintColor = CLAY_COLOR_TO_RAYLIB_COLOR(imageBgColor);
                // }

                // NOTE: .userData + .image = scroll image feature! If userData and imageData are both set, scroll the image.
                // If .userData is needed for something that also has an .image, modify these conditional safety checks.
                if (ScrollState* scrollState = static_cast<ScrollState*>(renderCommand->userData)) {
                    // match the parent ID or another "proxy" container ID with the current rendering ID.
                    // which allows its image to be controlled by the scroll vertical movement. This prevents accidental scrolling.
                    if (scrollState->parentId.id == renderCommand->id || scrollState->proxyId.id == renderCommand->id) {
                        // calculate the background image offset based on the vertical scroll position
                        // as the scroll position changes, render two backgrounds one after another
                        float imageOffset = scrollState->scrollY;
                        // WARNING: scrollY can be negative, wrap around
                        if (scrollState->scrollY + boundingBox.height <= 0) {
                            imageOffset = static_cast<float>(static_cast<int>(roundf(scrollState->scrollY)) % static_cast<int>(roundf(boundingBox.height)));
                        }
                        // NOTE: Scissor mode is used here to trim the extra image after doubling it for scrolling. 
                        // This prevents any sibling elements from rendering properly. The right approach would open the Scissor tag
                        // and then close it after all the siblings have done rendering...
                        BeginScissorMode(static_cast<int>(roundf(boundingBox.x)), static_cast<int>(roundf(boundingBox.y)), static_cast<int>(roundf(boundingBox.width)), static_cast<int>(roundf(boundingBox.height)));
                        DrawTexturePro(
                            texture,
                            Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                            Rectangle({boundingBox.x, boundingBox.y+imageOffset, boundingBox.width, boundingBox.height }),
                            Vector2({}), 0, WHITE);
                        DrawTexturePro(
                            texture,
                            Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                            Rectangle({boundingBox.x, boundingBox.y+boundingBox.height+imageOffset, boundingBox.width, boundingBox.height }),
                            Vector2({}), 0, WHITE);
                        EndScissorMode();
                    }
                } else {
                    DrawTexturePro(
                        texture,
                        Rectangle({ 0, 0, static_cast<float>(texture.width), static_cast<float>(texture.height) }),
                        Rectangle({boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }),
                        Vector2({}), 0, WHITE);
                }
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
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomRenderData *config = &renderCommand->renderData.custom;
                CustomLayoutElement *customElement = (CustomLayoutElement *)config->customData;
                if (!customElement) continue;
                switch (customElement->type) {
                    case CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL: {
                        Clay_BoundingBox rootBox = renderCommands.internalArray[0].boundingBox;
                        float scaleValue = CLAY__MIN(CLAY__MIN(1, 768 / rootBox.height) * CLAY__MAX(1, rootBox.width / 1024), 1.5f);
                        Ray positionRay = getScreenToWorldPointWithZDistance(Vector2({ renderCommand->boundingBox.x + renderCommand->boundingBox.width / 2, renderCommand->boundingBox.y + (renderCommand->boundingBox.height / 2) + 20 }), Raylib_camera, static_cast<int>(roundf(rootBox.width)), static_cast<int>(roundf(rootBox.height)), 140);
                        BeginMode3D(Raylib_camera);
                            DrawModel(customElement->customData.model.model, positionRay.position, customElement->customData.model.scale * scaleValue, WHITE);        // Draw 3d model with texture
                        EndMode3D();
                        break;
                    }
                    default: break;
                }
                break;
            }
            default: {
                printf("Error: unhandled render command.");
                exit(1);
            }
        }
    }
}


Action::Surface Surface::updateOptions(const InputEvent& inputEvent) {

    // update mouse for Clay
    // TODO: move to its own method?
    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(inputEvent.position);
    Clay_SetPointerState(mousePosition, inputEvent.id == Event::Input::PRIMARY || inputEvent.id == Event::Input::PRIMARY_DOWN);

    // check action before returning it to app
    // this function might not be called again
    auto action = widget.consumeButtonAction();

    // default mouse cursor before returning
    if (widget.onButtonJustHovered()) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (widget.onButtonJustBlurred() || action != 0) {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    return action;
}

Action::Surface Surface::updateMenu(const InputEvent& inputEvent) {

    // TODO: move to its own method?
    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(inputEvent.position);
    Clay_SetPointerState(mousePosition, inputEvent.id == Event::Input::PRIMARY || inputEvent.id == Event::Input::PRIMARY_DOWN);

    // update scrollbox
    // TODO: update based on scrollBoxId
    widget.updateScrollBox(inputEvent, mousePosition);

    // handle mouse cursor, if there is an action
    // this function might not be called again
    auto action = widget.consumeButtonAction();
    // default mouse cursor before returning
    if (widget.onButtonJustHovered()) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (widget.onButtonJustBlurred() || action != 0) {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    return action;
}

void Surface::updateDisplay(const GameState gameState) {
    if (gameState.score != gameScore) {
        gameScore = gameState.score;
        formatScore = std::format("Score {}", gameScore);
    }

    if (currentGameState != gameState.state) {
        currentGameState = gameState.state;
        if (gameState.state == State::Game::OVER || gameState.state == State::Game::WIN) {
            layoutMenu = &Surface::layoutWinLose;
            if (gameState.totalTimeId) {
                std::string totalTime = window.timer.consumeWatchTime(gameState.totalTimeId);
                formatTotalTime = std::format("Time {}", totalTime);
            }
        }
    }
}

void Surface::layoutWinLose() {
    CLAY(CLAY_ID("LayoutWinLose"), {
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(window.adapt(0.8f)),
                .height = CLAY_SIZING_PERCENT(window.adapt(0.8f)),
            },
            .childGap = static_cast<uint16_t>(window.scale(32)),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .floating = { 
            .offset = {0, 0}, 
            .zIndex = 1, 
            .attachPoints = { 
                CLAY_ATTACH_POINT_CENTER_CENTER, 
                CLAY_ATTACH_POINT_CENTER_CENTER 
            }, 
            .attachTo = CLAY_ATTACH_TO_PARENT 
        },
    }) {
        if (currentGameState == State::Game::WIN) {
            CLAY_TEXT(CLAY_STRING(TEXT_GAME_WIN_TITLE), STYLE_TEXT_WIN);
        } else if (currentGameState == State::Game::OVER) {
            CLAY_TEXT(CLAY_STRING(TEXT_GAME_LOSE_TITLE), STYLE_TEXT_LOSE);
        }

        CLAY(CLAY_ID("ContentWinLose"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(window.adapt(0.5f)),
                },
                .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(24))),
                .childGap = static_cast<uint16_t>(window.scale(4)),
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        }) { 
            if (currentGameState == State::Game::WIN) {
                CLAY_TEXT(CLAY_STRING(TEXT_GAME_WIN_SUBTITLE), STYLE_TEXT_DISPLAY);
            } else if (currentGameState == State::Game::OVER) {
                CLAY_TEXT(CLAY_STRING(TEXT_GAME_LOSE_SUBTITLE), STYLE_TEXT_DISPLAY);
            }

            widget.layoutLabel(formatScore);
            widget.layoutLabel(formatTotalTime);
        }

        CLAY(CLAY_ID("FooterWinLose"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(window.adapt(0.3f)),
                },
                .padding = { 0, 0, static_cast<uint16_t>(window.scale(12)), 0 },
            },
        }) {        
            widget.layoutButton(BUTTON_ID::RESTART);
        }
    }
}

void Surface::layoutTutorial() {

    CLAY(CLAY_ID("LayoutTutorial"), {
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(window.adapt(0.5f)),
                .height = CLAY_SIZING_PERCENT(0.7f),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = SURFACE_COLOR_MENU_BG,
        .cornerRadius = CLAY_CORNER_RADIUS(10),
        // example image that scrolls, this component Id
        // should be passed into to Widget.initScrollbar
        // .image = { .imageData = &textureBlueTile },
        .floating = { 
            .offset = {0, 0}, 
            .zIndex = 1, 
            .attachPoints = { 
                CLAY_ATTACH_POINT_CENTER_CENTER, 
                CLAY_ATTACH_POINT_CENTER_CENTER 
            }, 
            .attachTo = CLAY_ATTACH_TO_PARENT 
        },
        // scrollState.proxyId is layoutTutorialId
        // .userData = &widget.scrollState
    }) {
        // scrolling content
        widget.BeginScrollBox();
            CLAY_AUTO_ID({
                .layout = { 
                    .sizing = { .width = CLAY_SIZING_GROW(0) }, 
                    .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(20))), 
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, 
                }
            }) {
                CLAY_TEXT(CLAY_STRING(TEXT_TUTORIAL_TITLE), STYLE_TEXT_TITLE);
            }

            CLAY_TEXT(CLAY_STRING(TEXT_TUTORIAL_1), STYLE_TEXT_DEFAULT);
            CLAY_TEXT(CLAY_STRING(TEXT_TUTORIAL_2), STYLE_TEXT_DEFAULT);
            CLAY_TEXT(CLAY_STRING(TEXT_TUTORIAL_3), STYLE_TEXT_DEFAULT);
        widget.EndScrollBox();

        CLAY(CLAY_ID("FooterTutorial"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0),
                }, 
                .padding = { 
                    static_cast<uint16_t>(window.scale(120)),
                    static_cast<uint16_t>(window.scale(120)),
                    static_cast<uint16_t>(window.scale(32)),
                    static_cast<uint16_t>(window.scale(32))
                }, 
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        }) {        
            widget.layoutButton(BUTTON_ID::CONFIRM_TUTORIAL);
        }
    }
}

void Surface::layoutOptions() {
    // TODO: better styling
    CLAY(CLAY_ID("LayoutOptions"), {
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(window.adapt(0.5f)),
                .height = CLAY_SIZING_PERCENT(0.7f),
            }, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = SURFACE_COLOR_MENU_BG,
        .floating = { 
            .offset = {0, 0}, 
            .zIndex = 1, 
            .attachPoints = { 
                CLAY_ATTACH_POINT_CENTER_CENTER, 
                CLAY_ATTACH_POINT_CENTER_CENTER 
            }, 
            .attachTo = CLAY_ATTACH_TO_PARENT 
        },
    }) {

        CLAY_AUTO_ID({ 
            .layout = { 
                .sizing = { .width = CLAY_SIZING_GROW(0) }, 
                .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(20))), 
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, 
            }
        }) {
            CLAY_TEXT(CLAY_STRING(TEXT_OPTIONS_TITLE), STYLE_TEXT_TITLE);
        }

        CLAY_AUTO_ID({ 
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIXED(55.0f),
                }, 
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_BOTTOM },
                .layoutDirection = CLAY_LEFT_TO_RIGHT
            },
        }) {
            for (auto tabId : widget.tabButtonIds) {
                widget.layoutTab(tabId);
            }
        }

        CLAY(CLAY_ID("TabContentOptions"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                }, 
                .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(32))),
                .childGap = 16, 
                .layoutDirection = CLAY_TOP_TO_BOTTOM 
            },
            // NOTE: if options need scrollbar
            // .clip = { 
            //  .vertical = true, 
            //  .childOffset = Clay_GetScrollOffset()
            // },
        }) {
            BUTTON_ID activeTabId = widget.getActiveTab();
            if (activeTabId == BUTTON_ID::OPTIONS_GAME) {
                CLAY_TEXT(CLAY_STRING(TEXT_OPTIONS_TAB_TITLE_GAME), STYLE_TEXT_DEFAULT);
                // insert game options here
            } else if (activeTabId == BUTTON_ID::OPTIONS_AUDIO) {
                CLAY_TEXT(CLAY_STRING(TEXT_OPTIONS_TAB_TITLE_AUDIO), STYLE_TEXT_DEFAULT);
                // insert audio options here
            } else if (activeTabId == BUTTON_ID::OPTIONS_INPUTS) {
                CLAY_TEXT(CLAY_STRING(TEXT_OPTIONS_TAB_TITLE_INPUTS), STYLE_TEXT_DEFAULT);
                // insert input options here
            }
        }

        CLAY(CLAY_ID("FooterOptions"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_PERCENT(0.2f),
                }, 
                .padding = { 
                    static_cast<uint16_t>(window.scale(100)), 
                    static_cast<uint16_t>(window.scale(100)), 0, 0 },
                .childGap = static_cast<uint16_t>(window.scale(100)),
            },
        }) {      
            widget.layoutButtonSecondary(BUTTON_ID::CANCEL_OPTIONS);
            widget.layoutButtonSecondary(BUTTON_ID::CONFIRM_OPTIONS);
        }
    }
}

void Surface::layoutMenuPause() {
    CLAY(CLAY_ID("LayoutPauseMenu"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
        }, 
    }) {
        CLAY(CLAY_ID("ContentPauseMenu"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(window.adapt(0.33f)), 
                }, 
                .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(48))),
                .childGap = 2,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = SURFACE_COLOR_MENU_BG,
            .floating = { 
                .offset = {0, 0}, 
                .zIndex = 1, 
                .attachPoints = { 
                    CLAY_ATTACH_POINT_CENTER_CENTER, 
                    CLAY_ATTACH_POINT_CENTER_CENTER 
                }, 
                .attachTo = CLAY_ATTACH_TO_PARENT 
            },
        }) {

            if (surfaceEvent == Event::Surface::SHOW_RETURN_MAIN_MENU_CONFIRMATION) {
                CLAY(CLAY_ID("ModalReturnToMainWarning"), {
                    .layout = { 
                        .sizing = { 
                            .width = CLAY_SIZING_PERCENT(window.adapt(0.5f)), 
                        }, 
                        .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(42))),
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_BOTTOM },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = SURFACE_COLOR_ACCENT_BG,
                    .floating = { 
                        .offset = { 0, 0 },
                        .zIndex = 1, 
                        .attachPoints = { 
                            CLAY_ATTACH_POINT_CENTER_CENTER, 
                            CLAY_ATTACH_POINT_CENTER_CENTER 
                        }, 
                        .attachTo = CLAY_ATTACH_TO_ROOT 
                    },
                    .border = { 
                        .color = SURFACE_COLOR_ACCENT_BORDER, 
                        .width = CLAY_BORDER_OUTSIDE(6)
                    },
                    .transition = {
                        .handler = Clay_EaseOut,
                        .duration = 0.3f,
                        .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_POSITION | CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
                        .enter = { .setInitialState = FadeSlide },
                        // .exit = { .setFinalState = FadeSlide },
                    }
                }) {
                    CLAY(CLAY_ID("ContentReturnToMainWarning"), {
                        .layout = {
                            .sizing = { 
                                .width = CLAY_SIZING_GROW(0), 
                            },
                            .padding = CLAY_PADDING_ALL(static_cast<uint16_t>(window.scale(24))),
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                        },
                    }) {
                        // NOTES: CLAY_TEXT does not have .transition property, text cannot animate transition
                        // and as a result any fading on the parent leave the text unchange and looks jarring.
                        // solution is to either add .transition to each text element in Clay, or allow the parent
                        // to somehow force fade the children text nodes in it if a transition property is set.
                        CLAY_TEXT(CLAY_STRING(TEXT_RETURN_TO_MAIN), STYLE_TEXT_HIGHLIGHT);
                    }

                    CLAY(CLAY_ID("FooterReturnToMainWarning"), {
                        .layout = {
                            .sizing = { 
                                .width = CLAY_SIZING_GROW(0), 
                            },
                            .childGap = static_cast<uint16_t>(window.scale(48)),
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    }) {
                        widget.layoutButton(BUTTON_ID::CONFIRM_RETURN);
                        widget.layoutButton(BUTTON_ID::CANCEL_RETURN);
                    }
                }
            }

            CLAY_AUTO_ID({ 
                .layout = { 
                    .sizing = { .width = CLAY_SIZING_GROW(0) }, 
                    .padding = { 0, 0, 0, static_cast<uint16_t>(window.scale(32)) }, 
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, 
                }
            }) {
                CLAY_TEXT(CLAY_STRING("Paused"), STYLE_TEXT_TITLE);
            }

            for (auto buttonId : widget.buttonsMenuPause) {
                widget.layoutButton(buttonId);
            }
        }
    }
}

void Surface::layoutMenuMain() {
    CLAY(CLAY_ID("LayoutMainMenu"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
        }, 
    }) {
        CLAY(CLAY_ID("ContentMainMenu"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(0.33f), 
                    .height = CLAY_SIZING_PERCENT(0.5f) 
                }, 
                .childGap = 2,
                .layoutDirection = CLAY_TOP_TO_BOTTOM 
            },
            .floating = { 
                .offset = {0, -64*window.unit }, 
                .zIndex = 1, 
                .attachPoints = { 
                    CLAY_ATTACH_POINT_CENTER_CENTER, 
                    CLAY_ATTACH_POINT_CENTER_BOTTOM 
                }, 
                .attachTo = CLAY_ATTACH_TO_PARENT 
            },
            // TODO: in order for layout methods to be able to transition
            // they first need to be wrapped in a different function that gets called all the time
            // RESEARCH: is it viable to have one layout method called from app with a shell wrapper that calls
            // these other layout methods to allow transitioning?
            // .transition = {
            //     .handler = Clay_EaseOut,
            //     .duration = 0.3f,
            //     .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_DIMENSIONS | CLAY_TRANSITION_PROPERTY_POSITION | CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            //     .enter = { .setInitialState = FadeSlide },
            //     .exit = { .setFinalState = FadeSlide },
            // }
        }) {

            for (auto buttonId : widget.buttonsMenuMain) {
                widget.layoutButton(buttonId);
            }
        }
    }
}

// TODO: should probably be its own class like Widget
// would have components for HUD, and used here in a layout
// TODO: Layout could be also another class that handles doing the layout
void Surface::layoutDisplayGame() {
    CLAY(CLAY_ID("HUDContainer"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
            .padding = { 30, 40, 30, 30 }, 
        }, 
    }) {
        CLAY(CLAY_ID("LeftCounter"), {
            .layout = {
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0), 
                },
                .childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER }
            },
        }) {            
            widget.layoutLabel(formatScore);
        }

        // HUD Controls for Mobile
        CLAY(CLAY_ID("ControlHUD"), {
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED(210), 
                    .height = CLAY_SIZING_GROW(0),
                },
                .childAlignment = { .x = CLAY_ALIGN_X_RIGHT, .y = CLAY_ALIGN_Y_BOTTOM },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        }) {         

            CLAY(CLAY_ID("HUDControlsLeft"), {
                .layout = {
                    .sizing = { 
                        .width = CLAY_SIZING_FIXED(70), 
                        .height = CLAY_SIZING_FIXED(170), 
                    },
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                }
            }) {            

                widget.layoutButtonTexture(BUTTON_ID::MOVE_LEFT, &textureArrowLeft);
            }

            // TODO: get the fixed size dynamically from images
            CLAY(CLAY_ID("HUDControlsMiddle"), {
                .layout = {
                    .sizing = { 
                        .width = CLAY_SIZING_FIXED(70), 
                        .height = CLAY_SIZING_FIXED(170), 
                    },
                    .childGap = 30,
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                }
            }) {            
                widget.layoutButtonTexture(BUTTON_ID::MOVE_UP, &textureArrowUp);
                widget.layoutButtonTexture(BUTTON_ID::MOVE_DOWN, &textureArrowDown);
            }

            CLAY(CLAY_ID("HUDControlsRight"), {
                .layout = {
                    .sizing = { 
                        .width = CLAY_SIZING_FIXED(70), 
                        .height = CLAY_SIZING_FIXED(170), 
                    },
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                }
            }) {            
                widget.layoutButtonTexture(BUTTON_ID::MOVE_RIGHT, &textureArrowRight);
            }
        } // End HUD
    }
}

void Surface::beginEvent(Event::Surface event) {
    lastEvent = surfaceEvent;
    surfaceEvent = event;
}

void Surface::clearEvent() {
    surfaceEvent = lastEvent;
    lastEvent = Event::Surface::NO_EVENT;
}

bool Surface::hasEvent() const {
    // TODO: rename to activeEvent
    return surfaceEvent != Event::Surface::NO_EVENT;
}

void Surface::handleError(Clay_ErrorData errorData) {
    TraceLog(LOG_INFO, "%s", errorData.errorText.chars);
    Surface* self = static_cast<Surface*>(errorData.userData);

    if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        // reinitializeClay = true;
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        // reinitializeClay = true;
        Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    }

    uint64_t memorySize = Clay_MinMemorySize();
    void* memory = malloc(memorySize);
    self->arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    Clay_Initialize(self->arena, Clay_Dimensions({ self->window.widthf, self->window.heightf }), Clay_ErrorHandler({ .errorHandlerFunction = self->handleError, .userData = self }));
}

void Surface::resize(int width, int height) {
	Clay_SetLayoutDimensions(Clay_Dimensions({ static_cast<float>(width), static_cast<float>(height) }));
}

void Surface::transition(State::App appState, State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            if (surfaceEvent == Event::Surface::SHOW_OPTIONS) {
                layoutMenu  = &Surface::layoutOptions;
                update      = &Surface::updateOptions;
            } else {
                layoutMenu  = &Surface::layoutMenuMain;
                update      = &Surface::updateMenu;
            }
            layoutDisplay   = &Surface::layoutDisplayUnit;
            render          = &Surface::renderRaylib;
            break;
        case State::Screen::GAME:
            render = &Surface::renderRaylib;

            switch(appState) {
                case State::App::HOLD:
                    if (surfaceEvent == Event::Surface::SHOW_OPTIONS) {
                        layoutMenu  = &Surface::layoutOptions;
                        update      = &Surface::updateOptions;
                    } else {
                        layoutMenu  = &Surface::layoutMenuPause;
                        update      = &Surface::updateMenu;
                    }
                    layoutDisplay   = &Surface::layoutDisplayUnit;
                    break;
                case State::App::RUN:
                    if (surfaceEvent == Event::Surface::SHOW_TUTORIAL) {
                        layoutMenu = &Surface::layoutTutorial;
                    } else if (surfaceEvent == Event::Surface::SHOW_OPTIONS) {
                        layoutMenu = &Surface::layoutOptions;
                    } else {
                        // TODO: this implicitly assumes pause on APP::HOLD
                        // should there be explicit PAUSE state passed in
                        // or should updateMenu or equivalent set its own state?
                        layoutMenu = &Surface::layoutMenuUnit;
                    }
                    layoutDisplay   = &Surface::layoutDisplayGame;
                    update          = &Surface::updateMenu;
                    break;
                default:
                    layoutMenu      = &Surface::layoutMenuUnit;
                    layoutDisplay   = &Surface::layoutDisplayUnit;
                    update          = &Surface::updateUnit;
            }

            break;
        default:
            layoutMenu      = &Surface::layoutMenuUnit;
            layoutDisplay   = &Surface::layoutDisplayUnit;
            update          = &Surface::updateUnit;
            render          = &Surface::renderUnit;
    };
}

void Surface::unload(){

    UnloadTexture(textureArrowUp);
    UnloadTexture(textureArrowRight);
    UnloadTexture(textureArrowDown);
    UnloadTexture(textureArrowLeft);
    UnloadTexture(textureBlueTile);

    UnloadShader(overlayShader);
    
    for (auto& font : fonts) {
        UnloadFont(font);
    }

    if(temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer_len = 0;

    if(arena.memory) free(arena.memory);
}
