#define CLAY_IMPLEMENTATION
#include "surface.hpp"

#include "index.h"
#include "type.hpp"

#include "raylib.h"
#include "raymath.h"

#include <stdint.h>
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

    fonts[0] = LoadFontEx(PATH_ASSET("RobotoMono-Medium.ttf"), 48, 0, 400);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    fonts[1] = LoadFontEx(PATH_ASSET("Roboto-Regular.ttf"), 32, 0, 400);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    loadOverlay();

    // profilePicture = LoadTexture(PATH_ASSET("profile-picture.png"));
    // parchmentTexture = LoadTexture(PATH_ASSET("parchment.png"));
    // monkTexture = LoadTexture(PATH_ASSET("monk.png"));
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
    for (int j = 0; j < renderCommands.length; j++)
    {
        Clay_RenderCommand *renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = {renderCommand->boundingBox.x, renderCommand->boundingBox.y, renderCommand->boundingBox.width, renderCommand->boundingBox.height};
        switch (renderCommand->commandType)
        {
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
                Texture2D imageTexture = *(Texture2D *)renderCommand->renderData.image.imageData;
                Clay_Color tintColor = renderCommand->renderData.image.backgroundColor;
                if (tintColor.r == 0 && tintColor.g == 0 && tintColor.b == 0 && tintColor.a == 0) {
                    tintColor = Clay_Color({ 255, 255, 255, 255 });
                }

                // if userData and imageData are both set, the userData has scrollBar info
                // to calculate the offset for scrolling
                float imageOffset = 0.0f;
                if (renderCommand->userData) {
                    float scrollY = static_cast<ScrollbarData*>(renderCommand->userData)->scrollY;
                    if (scrollY + boundingBox.height <= 0) {
                        imageOffset = static_cast<float>(static_cast<int>(scrollY) % static_cast<int>(boundingBox.height));
                    } else {
                        imageOffset = scrollY;
                    }
                }

                DrawTexturePro(
                    imageTexture,
                    Rectangle({ 0, 0, static_cast<float>(imageTexture.width), static_cast<float>(imageTexture.height) }),
                    Rectangle({boundingBox.x, boundingBox.y+imageOffset, boundingBox.width, boundingBox.height }),
                    Vector2({}),
                    0,
                    CLAY_COLOR_TO_RAYLIB_COLOR(tintColor));
                // if userData is set, it is the scrollbarData info to render the second wrapping image 
                if (renderCommand->userData) {
                    DrawTexturePro(
                        imageTexture,
                        Rectangle({ 0, 0, static_cast<float>(imageTexture.width), static_cast<float>(imageTexture.height) }),
                        Rectangle({boundingBox.x, boundingBox.y+boundingBox.height+imageOffset, boundingBox.width, boundingBox.height }),
                        Vector2({}),
                        0,
                        CLAY_COLOR_TO_RAYLIB_COLOR(tintColor));
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
                // NOTE: this seems to convert Clay Color range to Raylib Color range first.
                // If Clay Color is used directly (which is also 4 floats), there is some flashing of white occurring.
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

static Clay_TransitionData ExitSlideUp(Clay_TransitionData initialState, Clay_TransitionProperty properties) {
    Clay_TransitionData targetState = initialState;
    if (properties & CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR) {
        targetState.overlayColor = Clay_Color({ 255, 255, 255, 200 });
    }
    if (properties & CLAY_TRANSITION_PROPERTY_HEIGHT) {
        // targetState.overlayColor = Clay_Color({ 255, 255, 255, 0 });
        targetState.boundingBox.height = 0.0f;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR) {
        targetState.backgroundColor.a = 0.0f;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_BORDER_COLOR) {
        targetState.borderColor.a = 0.0f;
    }
    return targetState;
}

Action::Surface Surface::updateMenu(const InputEvent& inputEvent) {

    // TODO: find a way to link this CLAY_ID to be used in update function
    // to pass through to Widget.updateScrollbar, which requires a reference to parent
    widget.updateScrollbar(inputEvent, Clay_GetElementId(CLAY_STRING("containerTutorial")));

    // handle mouse cursor, if there is an action
    // this function might not be called again
    // default mouse cursor before returning
    auto action = widget.consumeButtonAction();
    if (widget.onButtonJustHovered()) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else if (widget.onButtonJustBlurred() || action != 0) {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    return action;
}

void Surface::layoutTutorial() {

    CLAY(CLAY_ID("backgroundTutorial"), {
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(0.5f),
                .height = CLAY_SIZING_PERCENT(0.7f),
            }, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = SURFACE_MENU_COLOR_BG,
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
        // TODO: find a way to link this CLAY_ID to be used in update function
        // to pass through to Widget.updateScrollbar, which requires a reference to parent
        Clay_ElementId containerId = CLAY_ID("containerTutorial");
        CLAY(containerId, {
            .layout = { 
                .padding = { 24, 24, 48, 0 }, 
                .layoutDirection = CLAY_TOP_TO_BOTTOM 
            },
            .clip = { 
                .vertical = true, 
                .childOffset = Clay_GetScrollOffset()
            },

        }) {
            CLAY_TEXT(CLAY_STRING("Faucibus purus in massa tempor nec. Nec ullamcorper sit amet risus nullam eget felis eget nunc. Diam vulputate ut pharetra sit amet aliquam id diam. Lacus suspendisse faucibus interdum posuere lorem. A diam sollicitudin tempor id. Amet massa vitae tortor condimentum lacinia. Aliquet nibh praesent tristique magna."),
                CLAY_TEXT_CONFIG({ 
                    .textColor = CLAY_WHITE, 
                    .fontSize = 28,
                    .letterSpacing = 0, 
                    .lineHeight = 30,
                    .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                }));

            CLAY_TEXT(CLAY_STRING("Suspendisse in est ante in nibh. Amet venenatis urna cursus eget nunc scelerisque viverra. Elementum sagittis vitae et leo duis ut diam quam nulla. Enim nulla aliquet porttitor lacus. Pellentesque habitant morbi tristique senectus et. Facilisi nullam vehicula ipsum a arcu cursus vitae.\nSem fringilla ut morbi tincidunt. Euismod quis viverra nibh cras pulvinar mattis nunc sed. Velit sed ullamcorper morbi tincidunt ornare massa. Varius quam quisque id diam vel quam. Nulla pellentesque dignissim enim sit amet venenatis. Enim lobortis scelerisque fermentum dui faucibus in. Pretium viverra suspendisse potenti nullam ac tortor vitae. Lectus vestibulum mattis ullamcorper velit sed. Eget mauris pharetra et ultrices neque ornare aenean euismod elementum. Habitant morbi tristique senectus et. Integer vitae justo eget magna fermentum iaculis eu. Semper quis lectus nulla at volutpat diam. Enim praesent elementum facilisis leo. Massa vitae tortor condimentum lacinia quis vel."),
                CLAY_TEXT_CONFIG({ 
                    .textColor = CLAY_WHITE, 
                    .fontSize = 28,
                    .letterSpacing = 0, 
                    .lineHeight = 30,
                    .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                }));

            // WARNING: layoutScrollbar requires updateScrollbar call
            // during update phase (currently in updateMenu)
            widget.layoutScrollBar(containerId);
        }

        CLAY(CLAY_ID("containerTutorialConfirm"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_PERCENT(0.2f),
                }, 
                .padding = { 100, 100, 24, 0 }, 
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
        }) {        
            const Button& confirm = widget.getButton(BUTTON_ID::CONFIRM_TUTORIAL);
            widget.layoutButton(confirm.id, confirm.clayId, confirm.label);
        }
    }
}

void Surface::layoutOptions() {
    CLAY(CLAY_ID("containerOptions"), {
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(0.5f),
                .height = CLAY_SIZING_PERCENT(0.7f),
            }, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = SURFACE_MENU_COLOR_BG,
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

        CLAY_TEXT(CLAY_STRING("Options"), CLAY_TEXT_CONFIG({ 
                    .textColor = CLAY_WHITE, 
                    .fontSize = 28,
                    .letterSpacing = 0, 
                    .lineHeight = 30,
                    .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                }));

        CLAY(CLAY_ID("footerOptions"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_PERCENT(0.2f),
                }, 
             .padding = { 100, 100, 24, 0 }, 
            },
        }) {        
            const Button& confirm = widget.getButton(BUTTON_ID::CONFIRM_OPTIONS);
            const Button& cancel = widget.getButton(BUTTON_ID::CANCEL_OPTIONS);
            widget.layoutButton(confirm.id, confirm.clayId, confirm.label);
            widget.layoutButton(cancel.id, cancel.clayId, cancel.label);
        }
    }
}

void Surface::layoutMenuPause() {
    CLAY(CLAY_ID("ContainerPauseMenu"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
        }, 
        .backgroundColor = Clay_Color({ 0, 0, 0, 0 })
    }) {
        Clay_ElementId contentPauseMenuId = CLAY_ID("ContentPauseMenu");
        CLAY(contentPauseMenuId, {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(0.33f), 
                    .height = CLAY_SIZING_PERCENT(0.5f) 
                }, 
                .padding = { 16, 16, 16, 16 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = SURFACE_MENU_COLOR_BG,
            .floating = { 
                .offset = {0, 0}, 
                .zIndex = 1, 
                .attachPoints = { 
                    CLAY_ATTACH_POINT_CENTER_CENTER, 
                    CLAY_ATTACH_POINT_CENTER_CENTER 
                }, 
                .attachTo = CLAY_ATTACH_TO_PARENT 
            },
            .transition = {
                .handler = Clay_EaseOut,
                .duration = 0.3f,
                .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_DIMENSIONS | CLAY_TRANSITION_PROPERTY_POSITION | CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
                .enter = { .setInitialState = ExitSlideUp },
                .exit = { .setFinalState = ExitSlideUp },
            }
        }) {

            if (surfaceEvent == Event::Surface::SHOW_RETURN_MAIN_MENU_CONFIRMATION) {
                CLAY(CLAY_ID("PauseMenuConfirmationDialogue"), {
                    .layout = { 
                        .sizing = { 
                            .width = CLAY_SIZING_GROW(0), 
                            .height = CLAY_SIZING_PERCENT(0.5f)
                        }, 
                        .padding = { 16, 16, 16, 16 },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = { 140, 80, 200, 200 },
                    .floating = { 
                        .offset = {0, 0}, 
                        .zIndex = 1, 
                        .attachPoints = { 
                            CLAY_ATTACH_POINT_CENTER_CENTER, 
                            CLAY_ATTACH_POINT_CENTER_CENTER 
                        }, 
                        .attachTo = CLAY_ATTACH_TO_PARENT 
                    },
                    .border = { 
                        .color = Clay_Color({80, 80, 80, 255}), 
                        .width = CLAY_BORDER_OUTSIDE(2) 
                    },
                    .transition = {
                        .handler = Clay_EaseOut,
                        .duration = 0.3f,
                        .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_DIMENSIONS | CLAY_TRANSITION_PROPERTY_POSITION | CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
                        .enter = { .setInitialState = ExitSlideUp },
                        .exit = { .setFinalState = ExitSlideUp },
                    }
                }) {
                    CLAY(CLAY_ID("ContainerConfirmationText"), {
                        .layout = {
                            .sizing = { 
                                .width = CLAY_SIZING_GROW(0), 
                            },
                            .padding = { 20, 20, 20, 20 },
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                        },
                    }) {
                        // NOTES: CLAY_TEXT does not have .transition property, text cannot animate transition
                        // and as a result any fading on the parent leave the text unchange and looks jarring.
                        // solution is to either add .transition to each text element in Clay, or allow the parent
                        // to somehow force fade the children text nodes in it if a transition property is set.
                        CLAY_TEXT(CLAY_STRING("All progress will be lost. Return to Main Menu?"), CLAY_TEXT_CONFIG({ 
                            .textColor = Clay_Color({255,255,255,255}),
                            .fontSize = 24,
                        }));
                    }

                    CLAY(CLAY_ID("ContainerConfirmationButtons"), {
                        .layout = {
                            .sizing = { 
                                .width = CLAY_SIZING_GROW(0), 
                            },
                            .childGap = 20,
                            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                            .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        },
                    }) {
                        // TODO: overload createbutton to take button id
                        const Button& confirm = widget.getButton(BUTTON_ID::CONFIRM_RETURN);
                        const Button& cancel = widget.getButton(BUTTON_ID::CANCEL_RETURN);
                        widget.layoutButton(confirm.id, confirm.clayId, confirm.label);
                        widget.layoutButton(cancel.id, cancel.clayId, cancel.label);
                    }
                }
            }

            CLAY_TEXT(CLAY_STRING("Pause Menu"), CLAY_TEXT_CONFIG({ 
                .textColor = SURFACE_BUTTON_COLOR_FG,
                .fontSize = 24,
            }));

            for (auto buttonId : widget.buttonsMenuPause) {
                const Button& button = widget.getButton(buttonId);
                widget.layoutButton(button.id, button.clayId, button.label);
            }
        }
    }
}

void Surface::layoutMenuMain() {
    CLAY(CLAY_ID("ContainerMainMenu"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
        }, 
        .backgroundColor = Clay_Color({ 0, 0, 0, 0 })
    }) {
        Clay_ElementId contentMainMenuId = CLAY_ID("ContentMainMenu");
        CLAY(contentMainMenuId, {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(0.33f), 
                    .height = CLAY_SIZING_PERCENT(0.5f) 
                }, 
                .padding = { 16, 16, 16, 16 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM 
            },
            .backgroundColor = SURFACE_MENU_COLOR_BG,
            .floating = { 
                .offset = {0, 0}, 
                .zIndex = 1, 
                .attachPoints = { 
                    CLAY_ATTACH_POINT_CENTER_CENTER, 
                    CLAY_ATTACH_POINT_CENTER_CENTER 
                }, 
                .attachTo = CLAY_ATTACH_TO_PARENT 
            },
            .transition = {
                .handler = Clay_EaseOut,
                .duration = 0.3f,
                .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_DIMENSIONS | CLAY_TRANSITION_PROPERTY_POSITION | CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
                .enter = { .setInitialState = ExitSlideUp },
                .exit = { .setFinalState = ExitSlideUp },
            }
        }) {

            CLAY_TEXT(CLAY_STRING("Main Menu"), CLAY_TEXT_CONFIG({ 
                .textColor = SURFACE_BUTTON_COLOR_FG,
                .fontSize = 24,
            }));

            for (auto buttonId : widget.buttonsMenuMain) {
                const Button& button = widget.getButton(buttonId);
                widget.layoutButton(button.id, button.clayId, button.label);
            }
        }
    }
}

void Surface::layoutDisplayGame(GameState gameState) {
    CLAY(CLAY_ID("HUDContainer"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                // .height = CLAY_SIZING_GROW(0) 
            }, 
            .padding = { 16, 16, 16, 16 }, 
            .childGap = 16
        }, 
        .backgroundColor = Clay_Color({ 200, 200, 200, 0 })
    }) {
        CLAY(CLAY_ID("LeftCounter"), {
            .layout = {
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0), 
                },
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
            },
        }) {            
            // std::string&& numClicks = std::format("COUNTER: {}", gameState.raylibLogoClicks);
            const char* numClicksText = TextFormat("COUNTER: %d", 10);
            Clay_String numClicksClayString = CLAY__INIT(Clay_String){ .isStaticallyAllocated = true, .length = static_cast<int32_t>(strlen(numClicksText)), .chars = numClicksText };
            // CLAY_TEXT(Clay__IntToString(gameState.raylibLogoClicks), CLAY_TEXT_CONFIG({ 
            CLAY_TEXT(numClicksClayString, CLAY_TEXT_CONFIG({
                .textColor = Clay_Color({255,255,255,255}),
                .fontSize = 48,
            }));
        }

        CLAY(CLAY_ID("CenterCounter"), {
            .layout = {
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0), 
                }, 
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
            },
        }) {    
            const char* numBouncesText = TextFormat("BOUNCES: %d", 10);
            Clay_String numBouncesClayString = CLAY__INIT(Clay_String){ .isStaticallyAllocated = true, .length = static_cast<int32_t>(strlen(numBouncesText)), .chars = numBouncesText };
            // CLAY_TEXT(Clay__IntToString(gameState.raylibLogoClicks), CLAY_TEXT_CONFIG({ 
            CLAY_TEXT(numBouncesClayString, CLAY_TEXT_CONFIG({ 
                .textColor = Clay_Color({255,255,255,255}),
                .fontSize = 48,
            }));
        }

        CLAY(CLAY_ID("RightCounter"), {
            .layout = {
                .sizing = { 
                    .width = CLAY_SIZING_GROW(0), 
                }, 
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
            },
        }) {    
            const char* numCornersText = TextFormat("CORNERS: %d", 10);
            Clay_String numCornersClayString = CLAY__INIT(Clay_String){ .isStaticallyAllocated = true, .length = static_cast<int32_t>(strlen(numCornersText)), .chars = numCornersText };
            // CLAY_TEXT(Clay__IntToString(gameState.raylibLogoClicks), CLAY_TEXT_CONFIG({ 
            CLAY_TEXT(numCornersClayString, CLAY_TEXT_CONFIG({ 
                .textColor = Clay_Color({255,255,255,255}),
                .fontSize = 48,
            }));
        }
    }
}

// void Surface::layout(GameState gameState) {

//     CLAY(CLAY_ID("OuterContainer"), { 
//     	.layout = { 
//     		.sizing = { 
//     			.width = CLAY_SIZING_GROW(0), 
//     			.height = CLAY_SIZING_GROW(0) 
//     		}, 
//     		.padding = { 16, 16, 16, 16 }, 
//     		.childGap = 16 
//     	}, 
//     	.backgroundColor = Clay_Color({ 200, 200, 200, 0 })
//     }) {
//         Clay_ElementId sidebarId = CLAY_ID("SideBar");
//         CLAY(sidebarId, { 
//         	.layout = { 
//         		.sizing = { 
//         			.width = sidebarWidth, 
//         			.height = CLAY_SIZING_GROW(0) 
//         		}, 
//         		.padding = { 16, 16, 16, 16 }, 
//         		.childGap = 16, 
//         		.layoutDirection = CLAY_TOP_TO_BOTTOM 
//         	}, 
//         	.backgroundColor = Clay_Color({ 150, 150, 255, 255 }) 
//         }) {
//             CLAY(CLAY_ID("ProfilePictureOuter"), { 
//             	.layout = { 
//             		.sizing = { 
//             			.width = CLAY_SIZING_GROW(0) 
//             		}, 
//             		.padding = { 8, 8, 8, 8 }, 
//             		.childGap = 8, 
//             		.childAlignment = { .y = CLAY_ALIGN_Y_CENTER } 
//             	}, 
//             	.backgroundColor = {130, 130, 255, 255}, 
//             }) {
//                 CLAY(CLAY_ID("ProfilePicture"), { 
//                 	.layout = { 
//                 		.sizing = { 
//                 			.width = CLAY_SIZING_FIXED(60), 
//                 			.height = CLAY_SIZING_FIXED(60) 
//                 		},
//                		},
//             		.image = { .imageData = &profilePicture }, 
//                 });
//                 CLAY_TEXT(CLAY_STRING("Blah blah bla blahhasoa lalalsl"), CLAY_TEXT_CONFIG({ 
//                 	.textColor = Clay_Color({0, 0, 0, 255}), 
//                 	.fontSize = 24, 
//                 	.textAlignment = CLAY_TEXT_ALIGN_RIGHT 
//                 }));
//             }

//             if (Clay_Hovered() && buttonHoverId != sidebarId.id) {
//                 buttonHoverId = sidebarId.id;
//                 SetMouseCursor(MOUSE_CURSOR_DEFAULT);
//             }
            
//             buttonSimple(CLAY_ID("Bweh1"), CLAY_STRING("Show Overlay"));
//             buttonSimple(CLAY_ID("Bweh2"), CLAY_STRING("Some Other Item"));
//             buttonSimple(CLAY_ID("Bweh3"), CLAY_STRING("Another Item"));
//             buttonSimple(CLAY_ID("Bweh4"), CLAY_STRING("More Items"));

//         }


//         Clay_ElementId mainContentId = CLAY_ID("MainContent");
//         CLAY(mainContentId, { 
//         	.layout = { 
//         		.sizing = { 
//         			.width = CLAY_SIZING_GROW(0), 
//         			.height = CLAY_SIZING_GROW(0) 
//         		}, 
//         		.childGap = 16, 
//         		.layoutDirection = CLAY_TOP_TO_BOTTOM
//         	},
//     		// .backgroundColor = Clay_Color({ 200, 200, 200, 255 }),
//             .image = { .imageData = &parchmentTexture },
//             // pass the scrollbarData for the background image to scroll
//             .userData = &scrollbarData,
//         }) {
//             Clay_ElementId tabId1 = CLAY_ID("Tab1");
//             Clay_ElementId tabId2 = CLAY_ID("Tab2");
//             Clay_ElementId tabId3 = CLAY_ID("Tab3");

//             if (Clay_Hovered() && buttonHoverId != mainContentId.id) {
//                 buttonHoverId = mainContentId.id;
//                 SetMouseCursor(MOUSE_CURSOR_DEFAULT);
//             }

//             CLAY_AUTO_ID({ 
//             	.layout = { 
//             		.sizing = { 
//             			.width = CLAY_SIZING_GROW(0) 
//             		}, 
//         			.padding = {8, 8, 8, 8 }, 
//         			.childGap = 8, 
//         			.childAlignment = { .x = CLAY_ALIGN_X_RIGHT } 
//         		}, 
//             	.backgroundColor =  {180, 180, 180, 255} 
//         	}) {
// 	            buttonTab(tabId1, CLAY_STRING("Header Item 1"));
// 	            buttonTab(tabId2, CLAY_STRING("Header Item 2"));
// 	            buttonTab(tabId3, CLAY_STRING("Header Item 3"));
//             }

//             if (showOverlay) {                  
//                 CLAY(CLAY_ID("FloatingContainer"), {
//                     .layout = { 
//                         .sizing = { 
//                             .width = CLAY_SIZING_PERCENT(0.5f), 
//                             .height = CLAY_SIZING_PERCENT(0.2f) 
//                             // .width = CLAY_SIZING_GROW(0), 
//                             // .height = CLAY_SIZING_GROW(0) 
//                         }, 
//                         .padding = { 16, 16, 16, 16 }
//                     },
//                     .backgroundColor = { 140, 80, 200, 200 },
//                     .floating = { 
//                         .offset = {0, 0}, 
//                         .zIndex = 1, 
//                         .attachPoints = { 
//                             CLAY_ATTACH_POINT_CENTER_CENTER, 
//                             CLAY_ATTACH_POINT_CENTER_CENTER 
//                         }, 
//                         .attachTo = CLAY_ATTACH_TO_PARENT 
//                     },
//                     .border = { 
//                         .color = Clay_Color({80, 80, 80, 255}), 
//                         .width = CLAY_BORDER_OUTSIDE(2) 
//                     },
//                     .transition = {
//                         .handler = Clay_EaseOut,
//                         .duration = 0.3f,
//                         .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_DIMENSIONS | CLAY_TRANSITION_PROPERTY_POSITION | CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
//                         .enter = { .setInitialState = ExitSlideUp },
//                         .exit = { .setFinalState = ExitSlideUp },
//                     }
//                 }) {
//                     // NOTES: CLAY_TEXT does not have .transition property, text cannot animate transition
//                     // and as a result any fading on the parent leave the text unchange and looks jarring.
//                     // solution is to either add .transition to each text element in Clay, or allow the parent
//                     // to somehow force fade the children text nodes in it if a transition property is set.
//                     CLAY_TEXT(CLAY_STRING("I'm an inline floating container."), CLAY_TEXT_CONFIG({ 
//                         .textColor = Clay_Color({0,0,0,255}),
//                         .fontSize = 24,
//                     }));
//                 }
//             }

//             CLAY(CLAY_ID("TabContent"), {
//                 .layout = { 
//                 	.sizing = { 
//                 		.width = CLAY_SIZING_GROW(0),
//                         .height = CLAY_SIZING_GROW(0),
//                 	}, 
//                 	.padding = { 72, 72, 72, 72 }, 
//                 	.childGap = 16, 
//                 	.layoutDirection = CLAY_TOP_TO_BOTTOM 
//                 },
//                 // .backgroundColor = {200, 200, 255, 0},
//                 // .image = { .imageData = &parchmentTexture },
//                 .clip = { 
//                 	.vertical = true, 
//                 	.childOffset = Clay_GetScrollOffset()
//                 },

//             }) {


//                 if (activeTabId == tabId1.id) {     
//                     CLAY(CLAY_ID("MonkFrame"), { 
//                         .layout = { 
//                             .sizing = { 
//                                 .width = CLAY_SIZING_GROW(0) 
//                             }, 
//                             .padding = { 8, 8, 8, 8 }, 
//                             .childGap = 8, 
//                             .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } 
//                         }, 
//                         // .backgroundColor = {130, 130, 255, 255}, 
//                     }) {
//                         CLAY(CLAY_ID("MonkPicture"), { 
//                             .layout = { 
//                                 .sizing = { 
//                                     .width = CLAY_SIZING_FIXED(256), 
//                                     .height = CLAY_SIZING_FIXED(256) 
//                                 },
//                             },
//                             .image = { .imageData = &monkTexture }, 
//                         });
//                          CLAY_TEXT(CLAY_STRING("VAE VICTIS"),
//                              CLAY_TEXT_CONFIG({ 
//                                 .textColor = {0,0,0,200}, 
//                                 .fontSize = 164,
//                                 .letterSpacing = 0, 
//                                 .lineHeight = 100,
//                                 .textAlignment = CLAY_TEXT_ALIGN_LEFT 
//                             }));
//                     }               
//                      CLAY_TEXT(CLAY_STRING("Amet cursus sit amet dictum sit amet justo donec. Et malesuada fames ac turpis egestas maecenas. A lacus vestibulum sed arcu non odio euismod lacinia. Gravida neque convallis a cras. Dui nunc mattis enim ut tellus elementum sagittis vitae et. Orci sagittis eu volutpat odio facilisis mauris. Neque gravida in fermentum et sollicitudin ac orci. Ultrices dui sapien eget mi proin sed libero. Euismod quis viverra nibh cras pulvinar mattis. Diam volutpat commodo sed egestas egestas. In fermentum posuere urna nec tincidunt praesent semper. Integer eget aliquet nibh praesent tristique magna.\nId cursus metus aliquam eleifend mi in. Sed pulvinar proin gravida hendrerit lectus a. Etiam tempor orci eu lobortis elementum nibh tellus. Nullam vehicula ipsum a arcu cursus vitae. Elit scelerisque mauris pellentesque pulvinar pellentesque habitant morbi tristique senectus. Condimentum lacinia quis vel eros donec ac odio. Mattis pellentesque id nibh tortor id aliquet lectus. Turpis egestas integer eget aliquet nibh praesent tristique. Porttitor massa id neque aliquam vestibulum morbi. Mauris commodo quis imperdiet massa tincidunt nunc pulvinar sapien et. Nunc scelerisque viverra mauris in aliquam sem fringilla. Suspendisse ultrices gravida dictum fusce ut placerat orci nulla.\nLacus laoreet non curabitur gravida arcu ac tortor dignissim. Urna nec tincidunt praesent semper feugiat nibh sed pulvinar. Tristique senectus et netus et malesuada fames ac. Nunc aliquet bibendum enim facilisis gravida. Egestas maecenas pharetra convallis posuere morbi leo urna molestie. Sapien nec sagittis aliquam malesuada bibendum arcu vitae elementum curabitur. Ac turpis egestas maecenas pharetra convallis posuere morbi leo urna. Viverra vitae congue eu consequat. Aliquet enim tortor at auctor urna. Ornare massa eget egestas purus viverra accumsan in nisl nisi. Elit pellentesque habitant morbi tristique senectus et netus et malesuada.\nSuspendisse ultrices gravida dictum fusce ut placerat orci nulla pellentesque. Lobortis feugiat vivamus at augue eget arcu. Vitae justo eget magna fermentum iaculis eu. Gravida rutrum quisque non tellus orci. Ipsum faucibus vitae aliquet nec. Nullam non nisi est sit amet. Nunc consequat interdum varius sit amet mattis vulputate enim. Sem fringilla ut morbi tincidunt augue interdum. Vitae purus faucibus ornare suspendisse. Massa tincidunt nunc pulvinar sapien et. Fringilla ut morbi tincidunt augue interdum velit euismod in. Donec massa sapien faucibus et. Est placerat in egestas erat imperdiet. Gravida rutrum quisque non tellus. Morbi non arcu risus quis varius quam quisque id diam. Habitant morbi tristique senectus et netus et malesuada fames ac. Eget lorem dolor sed viverra.\nOrnare massa eget egestas purus viverra. Varius vel pharetra vel turpis nunc eget lorem. Consectetur purus ut faucibus pulvinar elementum. Placerat in egestas erat imperdiet sed euismod nisi. Interdum velit euismod in pellentesque massa placerat duis ultricies lacus. Aliquam nulla facilisi cras fermentum odio eu. Est pellentesque elit ullamcorper dignissim cras tincidunt. Nunc sed id semper risus in hendrerit gravida rutrum. A pellentesque sit amet porttitor eget dolor morbi. Pellentesque habitant morbi tristique senectus et netus et malesuada fames. Nisl nunc mi ipsum faucibus vitae aliquet nec ullamcorper. Sed id semper risus in hendrerit gravida. Tincidunt praesent semper feugiat nibh. Aliquet lectus proin nibh nisl condimentum id venenatis a. Enim sit amet venenatis urna cursus eget. In egestas erat imperdiet sed euismod nisi porta lorem mollis. Lacinia quis vel eros donec ac odio tempor orci. Donec pretium vulputate sapien nec sagittis aliquam malesuada bibendum arcu. Erat pellentesque adipiscing commodo elit at.\nEgestas sed sed risus pretium quam vulputate. Vitae congue mauris rhoncus aenean vel elit scelerisque mauris pellentesque. Aliquam malesuada bibendum arcu vitae elementum. Congue mauris rhoncus aenean vel elit scelerisque mauris. Pellentesque dignissim enim sit amet venenatis urna cursus. Et malesuada fames ac turpis egestas sed tempus urna. Vel fringilla est ullamcorper eget nulla facilisi etiam dignissim. Nibh cras pulvinar mattis nunc sed blandit libero. Fringilla est ullamcorper eget nulla facilisi etiam dignissim. Aenean euismod elementum nisi quis eleifend quam adipiscing vitae proin. Mauris pharetra et ultrices neque ornare aenean euismod elementum. Ornare quam viverra orci sagittis eu. Odio ut sem nulla pharetra diam sit amet nisl suscipit. Ornare lectus sit amet est. Ullamcorper sit amet risus nullam eget. Tincidunt lobortis feugiat vivamus at augue eget arcu dictum.\nUrna nec tincidunt praesent semper feugiat nibh. Ut venenatis tellus in metus vulputate eu scelerisque felis. Cursus risus at ultrices mi tempus. In pellentesque massa placerat duis ultricies lacus sed turpis. Platea dictumst quisque sagittis purus. Cras adipiscing enim eu turpis egestas. Egestas sed tempus urna et pharetra pharetra. Netus et malesuada fames ac turpis egestas integer eget aliquet. Ac turpis egestas sed tempus. Sed lectus vestibulum mattis ullamcorper velit sed. Ante metus dictum at tempor commodo ullamcorper a. Augue neque gravida in fermentum et sollicitudin ac. Praesent semper feugiat nibh sed pulvinar proin gravida. Metus aliquam eleifend mi in nulla posuere sollicitudin aliquam ultrices. Neque gravida in fermentum et sollicitudin ac orci phasellus egestas.\nRidiculus mus mauris vitae ultricies. Morbi quis commodo odio aenean. Duis ultricies lacus sed turpis. Non pulvinar neque laoreet suspendisse interdum consectetur. Scelerisque eleifend donec pretium vulputate sapien nec sagittis aliquam. Volutpat est velit egestas dui id ornare arcu odio ut. Viverra tellus in hac habitasse platea dictumst vestibulum rhoncus est. Vestibulum lectus mauris ultrices eros. Sed blandit libero volutpat sed cras ornare. Id leo in vitae turpis massa sed elementum tempus. Gravida dictum fusce ut placerat orci nulla pellentesque. Pretium quam vulputate dignissim suspendisse in. Nisl suscipit adipiscing bibendum est ultricies integer quis auctor. Risus viverra adipiscing at in tellus. Turpis nunc eget lorem dolor sed viverra ipsum. Senectus et netus et malesuada fames ac. "),
//                          CLAY_TEXT_CONFIG({ 
//                             .textColor = {0,0,0,255}, 
//                             .fontSize = 48,
//                             .letterSpacing = 0, 
//                             .lineHeight = 30,
//                             .textAlignment = CLAY_TEXT_ALIGN_LEFT 
//                         }));
//                 } else if (activeTabId == tabId2.id) {  
//                     CLAY_TEXT(CLAY_STRING("Faucibus purus in massa tempor nec. Nec ullamcorper sit amet risus nullam eget felis eget nunc. Diam vulputate ut pharetra sit amet aliquam id diam. Lacus suspendisse faucibus interdum posuere lorem. A diam sollicitudin tempor id. Amet massa vitae tortor condimentum lacinia. Aliquet nibh praesent tristique magna."),
//                         CLAY_TEXT_CONFIG({ 
//                             .textColor = {0,0,0,255}, 
//                             .fontSize = 48,
//                             .letterSpacing = 0, 
//                             .lineHeight = 30,
//                             .textAlignment = CLAY_TEXT_ALIGN_LEFT 
//                         }));

//                     CLAY_TEXT(CLAY_STRING("Suspendisse in est ante in nibh. Amet venenatis urna cursus eget nunc scelerisque viverra. Elementum sagittis vitae et leo duis ut diam quam nulla. Enim nulla aliquet porttitor lacus. Pellentesque habitant morbi tristique senectus et. Facilisi nullam vehicula ipsum a arcu cursus vitae.\nSem fringilla ut morbi tincidunt. Euismod quis viverra nibh cras pulvinar mattis nunc sed. Velit sed ullamcorper morbi tincidunt ornare massa. Varius quam quisque id diam vel quam. Nulla pellentesque dignissim enim sit amet venenatis. Enim lobortis scelerisque fermentum dui faucibus in. Pretium viverra suspendisse potenti nullam ac tortor vitae. Lectus vestibulum mattis ullamcorper velit sed. Eget mauris pharetra et ultrices neque ornare aenean euismod elementum. Habitant morbi tristique senectus et. Integer vitae justo eget magna fermentum iaculis eu. Semper quis lectus nulla at volutpat diam. Enim praesent elementum facilisis leo. Massa vitae tortor condimentum lacinia quis vel."),
//                         CLAY_TEXT_CONFIG({ 
//                             .textColor = {0,0,0,255}, 
//                             .fontSize = 48,
//                             .letterSpacing = 0, 
//                             .lineHeight = 30,
//                             .textAlignment = CLAY_TEXT_ALIGN_LEFT 
//                         }));
                
//                 } if (activeTabId == tabId3.id) {  

//                      CLAY(CLAY_ID("Photos"), { 
//                         .layout = { 
//                             .sizing = { 
//                                 .width = CLAY_SIZING_GROW(0) 
//                             }, 
//                             .padding = {16, 16, 16, 16}, 
//                             .childGap = 16, 
//                             .childAlignment = { 
//                                 .x = CLAY_ALIGN_X_CENTER, 
//                                 .y = CLAY_ALIGN_Y_CENTER 
//                             } 
//                         }, 
//                         .backgroundColor = {180, 180, 220, 255},
//                         // .image = { .imageData = &parchmentTexture },
//                      }) {
//                          CLAY(CLAY_ID("Picture2"), { 
//                             .layout = { 
//                                 .sizing = { 
//                                     .width = CLAY_SIZING_FIXED(120) 
//                                 }
//                             }, 
//                             .overlayColor = { 100, 0, 0, 140 },
//                             .aspectRatio = { 0.5f },
//                             .image = { .imageData = &profilePicture },
//                          });
//                          CLAY(CLAY_ID("Picture1"), { 
//                             .layout = { 
//                                 .padding = {8, 8, 8, 8}, 
//                                 .childAlignment = { 
//                                     .x = CLAY_ALIGN_X_CENTER 
//                                 }, 
//                                 .layoutDirection = CLAY_TOP_TO_BOTTOM 
//                             }, 
//                             .backgroundColor = {170, 170, 220, 255} 
//                          }) {
//                              CLAY(CLAY_ID("ProfilePicture2"), { 
//                                 .layout = { 
//                                     .sizing = { 
//                                         .width = CLAY_SIZING_FIXED(60), 
//                                         .height = CLAY_SIZING_FIXED(60) 
//                                     }
//                                 },
//                                 .image = { .imageData = &profilePicture }, 
//                              });
//                              CLAY_TEXT(CLAY_STRING("Image caption below"), CLAY_TEXT_CONFIG({ .textColor = {0,0,0,255}, .fontSize = 24 }));
//                          }
//                          CLAY(CLAY_ID("Picture3"), { 
//                             .layout = { 
//                                 .sizing = { 
//                                     .width = CLAY_SIZING_FIXED(120) 
//                                 }
//                             }, 
//                             .aspectRatio = { 2.0f },
//                             .image = { .imageData = &profilePicture }, 
//                          });
//                      }

//                 } else if (activeTabId == 0) {
//                     activeTabId = tabId1.id;
//                 }


//             } // main content






//TODO: separate into ScrollBar method



// 	        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("TabContent")));
// 	        if (scrollData.found && scrollData.scrollContainerDimensions.height < scrollData.contentDimensions.height) {
// 	            CLAY(CLAY_ID("ScrollBar"), {
// 	                .floating = {
// 	                    .offset = { 
// 	                    	.y = -(scrollData.scrollPosition->y / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height 
// 	                    },
// 	                    .parentId = Clay_GetElementId(CLAY_STRING("TabContent")).id,
// 	                    .zIndex = 1,
// 	                    .attachPoints = { 
// 	                    	.element = CLAY_ATTACH_POINT_RIGHT_TOP, 
// 	                    	.parent = CLAY_ATTACH_POINT_RIGHT_TOP 
// 	                    },
// 	                    .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
// 	                }
// 	            }) {
// 	                CLAY(CLAY_ID("ScrollBarButton"), {
// 	                    .layout = { 
// 	                    	.sizing = { 
// 	                    		CLAY_SIZING_FIXED(12), 
// 	                    		CLAY_SIZING_FIXED((scrollData.scrollContainerDimensions.height / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height) 
// 	                    	}
// 	                    },
// 	                    .backgroundColor = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar"))) ? Clay_Color({100, 100, 140, 150}) : Clay_Color({120, 120, 160, 150}),
// 	                    .cornerRadius = CLAY_CORNER_RADIUS(6),
// 	                });
// 	            }
// 	        }
//         } // right panel


//     } // container

// }

void Surface::beginEvent(Event::Surface event) {
    lastEvent = surfaceEvent;
    surfaceEvent = event;
}

void Surface::clearEvent() {
    surfaceEvent = lastEvent;
    lastEvent = Event::Surface::NO_EVENT;
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
    // if (width < 720) {
    //     sidebarWidth = CLAY_SIZING_FIXED(150);
    // } else {
    //     sidebarWidth = CLAY_SIZING_PERCENT(0.2f);
    // }

	Clay_SetLayoutDimensions(Clay_Dimensions({ static_cast<float>(width), static_cast<float>(height) }));
}

void Surface::transition(State::App appState, State::Screen screen) {
    switch(screen) {
        case State::Screen::MAIN:
            if (surfaceEvent == Event::Surface::SHOW_OPTIONS) {
                layoutMenu  = &Surface::layoutOptions;
            } else {
                layoutMenu  = &Surface::layoutMenuMain;
            }
            layoutDisplay   = &Surface::layoutDisplayUnit;
            update          = &Surface::updateMenu;
            render          = &Surface::renderRaylib;
            break;
        case State::Screen::GAME:
            render = &Surface::renderRaylib;

            switch(appState) {
                case State::App::HOLD:
                    if (surfaceEvent == Event::Surface::SHOW_OPTIONS) {
                        layoutMenu  = &Surface::layoutOptions;
                    } else {
                        layoutMenu  = &Surface::layoutMenuPause;
                    }
                    layoutDisplay   = &Surface::layoutDisplayUnit;
                    update          = &Surface::updateMenu;
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
    if(temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer_len = 0;

    UnloadShader(overlayShader);
    UnloadFont(fonts[0]);
    UnloadFont(fonts[1]);

    free(arena.memory);

    // UnloadTexture(parchmentTexture);
    // UnloadTexture(profilePicture);
    // UnloadTexture(monkTexture);
}
