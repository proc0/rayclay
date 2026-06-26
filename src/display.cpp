#include "types.hpp"
#define CLAY_IMPLEMENTATION
#include "display.hpp"

#include "raylib.h"
#include "raymath.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "config.h"


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

void Display::load(){
    // 1. Query minimum memory required for default element limits
    uint64_t memorySize = Clay_MinMemorySize();
    // 2. Allocate memory (malloc, stack, or custom allocator)
    void* memory = malloc(memorySize);
    // 3. Create arena [clay.h:2150-2158]
    arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    // 4. Initialize Clay [clay.h:2186-2188]
    Clay_Initialize(arena, Clay_Dimensions({ static_cast<float>(screen.width()), static_cast<float>(screen.height()) }), Clay_ErrorHandler({ .errorHandlerFunction = handleError, .userData = this }));

    fonts[0] = LoadFontEx(PATH_ASSET("RobotoMono-Medium.ttf"), 48, 0, 400);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    fonts[1] = LoadFontEx(PATH_ASSET("Roboto-Regular.ttf"), 32, 0, 400);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    initOverlay();

    profilePicture = LoadTexture(PATH_ASSET("profile-picture.png"));
    parchmentTexture = LoadTexture(PATH_ASSET("parchment.png"));
    monkTexture = LoadTexture(PATH_ASSET("monk.png"));
}

void Display::renderNull(Clay_RenderCommandArray& renderCommands) const {
}

void Display::render(Clay_RenderCommandArray& renderCommands) const {
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
                // setColorOverlay(CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->renderData.overlayColor.color));
                Color color = CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->renderData.overlayColor.color);
			    float colorFloat[4] = {
			        static_cast<float>(color.r)/255.0f,
			        static_cast<float>(color.g)/255.0f,
			        static_cast<float>(color.b)/255.0f,
			        static_cast<float>(color.a)/255.0f,
			    };

			    SetShaderValue(overlayShader, colorLoc, colorFloat, SHADER_UNIFORM_VEC4);
			    BeginShaderMode(overlayShader);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END: {
                // disableColorOverlay();
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

void Display::initOverlay() {
	const char* overlayShaderCode = "#version 330\n"
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
	                                "\n"
	                                "    vec3 blendedRGB = mix(texelColor.rgb, overlayColor.rgb, overlayColor.a);\n"
	                                "\n"
	                                "    finalColor = vec4(blendedRGB, texelColor.a);\n"
	                                "}";
    overlayShader = LoadShaderFromMemory(0, overlayShaderCode);
    colorLoc = GetShaderLocation(overlayShader, "overlayColor");
}

// void Display::setColorOverlay(Color color) const {
//     overlayEnabled = true;
//     float colorFloat[4] = {
//         static_cast<float>(color.r)/255.0f,
//         static_cast<float>(color.g)/255.0f,
//         static_cast<float>(color.b)/255.0f,
//         static_cast<float>(color.a)/255.0f,
//     };

//     SetShaderValue(overlayShader, colorLoc, colorFloat, SHADER_UNIFORM_VEC4);
//     BeginShaderMode(overlayShader);
// }

// void Display::disableColorOverlay() const {
//     if (overlayEnabled) {
//         EndShaderMode();
//         overlayEnabled = false;
//     } else {
//     	TraceLog(LOG_INFO, "OVERLAY HIT");
//     }
// }

void handleButtonClick(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Display* display = static_cast<Display*>(userData);
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        std::string idStr(elementId.stringId.chars);
    	display->buttonAction = display->buttonActions.at(idStr.c_str());
    }
}

void Display::buttonSimple(const Clay_ElementId& elementId, const Clay_String& buttonText, Action::Display action) {
	// Clay_Color bgColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE);

    CLAY(elementId, { 
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER } 
        }, 
        .backgroundColor = Clay_Hovered() ? Clay_Color({ 125, 125, 125, 255 }) : Clay_Color({ 150, 150, 150, 255 }),
        .border = { 
            .color = Clay_Color({ 255, 255, 255, 255 }), 
            .width = CLAY_BORDER_OUTSIDE(2) 
        },
    }) {
        Clay_Color textColor = { 220, 220, 220, 255 };
        if (Clay_Hovered() && buttonHoverId != elementId.id) {
            buttonHoverId = elementId.id;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            textColor = { 255, 255, 255, 255 };
        }
    	Clay_OnHover(handleButtonClick, this);
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({ .textColor = textColor, .fontSize = 24 }));
    }
}


void handleButtonTabClick(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Display* display = static_cast<Display*>(userData);
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        display->activeTabId = elementId.id;
    }
}

void Display::buttonTab(const Clay_ElementId& elementId, const Clay_String& buttonText) {
    CLAY(elementId, { 
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER } 
        }, 
        .backgroundColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE),
    }) {
        if (Clay_Hovered() && buttonHoverId != elementId.id) {
            buttonHoverId = elementId.id;
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        }
        Clay_OnHover(handleButtonTabClick, this);
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({ .textColor = { 255, 255, 255, 255 }, .fontSize = 24 }));
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

// static Clay_TransitionData TransitionFadeOut(Clay_TransitionData initialState, Clay_TransitionProperty properties) {
//     Clay_TransitionData targetState = initialState;
//     if (properties & CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR) {
//         targetState.backgroundColor.a = 0.0f;
//     }
//     if (properties & CLAY_TRANSITION_PROPERTY_BORDER_COLOR) {
//         targetState.borderColor.a = 0.0f;
//     }
//     return targetState;
// }

Action::Display Display::updateNull(const InputEvent& inputEvent) {
    return Action::Display::DO_NOTHING;
}

Action::Display Display::update(const InputEvent& inputEvent) {

    bool isMouseDown = inputEvent.id == Event::Input::PRIMARY || inputEvent.id == Event::Input::PRIMARY_DOWN;
    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mousePosition, isMouseDown && !scrollbarData.mouseDown);

    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;
    Clay_UpdateScrollContainers(true, Clay_Vector2({ mouseWheelX, mouseWheelY }), GetFrameTime());

    if (inputEvent.id == Event::Input::PRIMARY_UP) {
        scrollbarData.mouseDown = false;
    }

        Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("TabContent")));
    if (isMouseDown && !scrollbarData.mouseDown && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar")))) {
        scrollbarData.clickOrigin = mousePosition;
        scrollbarData.positionOrigin = *scrollContainerData.scrollPosition;
        scrollbarData.mouseDown = true;

    } else if (scrollbarData.mouseDown) {
        // Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("TabContent")));
        if (scrollContainerData.contentDimensions.height > 0) {
            Clay_Vector2 ratio = Clay_Vector2({
                scrollContainerData.contentDimensions.width / scrollContainerData.scrollContainerDimensions.width,
                scrollContainerData.contentDimensions.height / scrollContainerData.scrollContainerDimensions.height,
            });

            if (scrollContainerData.config.vertical) {
                scrollContainerData.scrollPosition->y = scrollbarData.positionOrigin.y + (scrollbarData.clickOrigin.y - mousePosition.y) * ratio.y;
            }
            
            if (scrollContainerData.config.horizontal) {
                scrollContainerData.scrollPosition->x = scrollbarData.positionOrigin.x + (scrollbarData.clickOrigin.x - mousePosition.x) * ratio.x;
            }
        }
    }

    if(scrollContainerData.scrollPosition) {

        scrollbarData.scrollY = scrollContainerData.scrollPosition->y - scrollbarData.positionOrigin.y;
        // TraceLog(LOG_INFO, "scroll %f", scrollbarData.scrollY);
    }

    // Action::Display lastButtonAction = buttonAction;
    // buttonAction = Action::Display::DO_NOTHING;

    return buttonAction;
}

void Display::layoutPauseMenu() {
    CLAY(CLAY_ID("ContainerPauseMenu"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
        }, 
        .backgroundColor = Clay_Color({ 0, 0, 0, 0 })
    }) {
        CLAY(CLAY_ID("ContentPauseMenu"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(0.33f), 
                    .height = CLAY_SIZING_PERCENT(0.5f) 
                }, 
                .padding = { 16, 16, 16, 16 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = { 0, 0, 0, 200 },
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
            // NOTES: CLAY_TEXT does not have .transition property, text cannot animate transition
            // and as a result any fading on the parent leave the text unchange and looks jarring.
            // solution is to either add .transition to each text element in Clay, or allow the parent
            // to somehow force fade the children text nodes in it if a transition property is set.
            CLAY_TEXT(CLAY_STRING("Pause Menu"), CLAY_TEXT_CONFIG({ 
                .textColor = Clay_Color({ 150, 150, 150, 255 }),
                .fontSize = 24,
            }));

            buttonSimple(CLAY_ID("ButtonGameResume"), CLAY_STRING("Resume Game"), Action::Display::RESUME_GAME);
            buttonSimple(CLAY_ID("ButtonGameLoad"), CLAY_STRING("Load Game"), Action::Display::LOAD_GAME);
            buttonSimple(CLAY_ID("ButtonOptions"), CLAY_STRING("Options"), Action::Display::OPTIONS);
            buttonSimple(CLAY_ID("ButtonMainMenu"), CLAY_STRING("Main Menu"), Action::Display::MAIN_MENU);
            buttonSimple(CLAY_ID("ButtonQuit"), CLAY_STRING("Quit"), Action::Display::QUIT_APP);
        }
    }

    buttonAction = Action::Display::DO_NOTHING;
}

void Display::layoutMainMenu() {
    CLAY(CLAY_ID("ContainerMainMenu"), { 
        .layout = { 
            .sizing = { 
                .width = CLAY_SIZING_GROW(0), 
                .height = CLAY_SIZING_GROW(0) 
            }, 
        }, 
        .backgroundColor = Clay_Color({ 0, 0, 0, 0 })
    }) {
        CLAY(CLAY_ID("ContentMainMenu"), {
            .layout = { 
                .sizing = { 
                    .width = CLAY_SIZING_PERCENT(0.33f), 
                    .height = CLAY_SIZING_PERCENT(0.5f) 
                }, 
                .padding = { 16, 16, 16, 16 },
                .childGap = 16,
                .layoutDirection = CLAY_TOP_TO_BOTTOM 
            },
            .backgroundColor = { 0, 0, 0, 200 },
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
            // NOTES: CLAY_TEXT does not have .transition property, text cannot animate transition
            // and as a result any fading on the parent leave the text unchange and looks jarring.
            // solution is to either add .transition to each text element in Clay, or allow the parent
            // to somehow force fade the children text nodes in it if a transition property is set.
            CLAY_TEXT(CLAY_STRING("Main Menu"), CLAY_TEXT_CONFIG({ 
                .textColor = Clay_Color({ 150, 150, 150, 255 }),
                .fontSize = 24,
            }));

            buttonSimple(CLAY_ID("ButtonGameNew"), CLAY_STRING("New Game"), Action::Display::NEW_GAME);
            buttonSimple(CLAY_ID("ButtonGameLoad"), CLAY_STRING("Load Game"), Action::Display::LOAD_GAME);
            buttonSimple(CLAY_ID("ButtonOptions"), CLAY_STRING("Options"), Action::Display::OPTIONS);
            buttonSimple(CLAY_ID("ButtonQuit"), CLAY_STRING("Quit"), Action::Display::QUIT_APP);
        }
    }

    buttonAction = Action::Display::DO_NOTHING;
}

void Display::layout() {

    CLAY(CLAY_ID("OuterContainer"), { 
    	.layout = { 
    		.sizing = { 
    			.width = CLAY_SIZING_GROW(0), 
    			.height = CLAY_SIZING_GROW(0) 
    		}, 
    		.padding = { 16, 16, 16, 16 }, 
    		.childGap = 16 
    	}, 
    	.backgroundColor = Clay_Color({ 200, 200, 200, 0 })
    }) {
        Clay_ElementId sidebarId = CLAY_ID("SideBar");
        CLAY(sidebarId, { 
        	.layout = { 
        		.sizing = { 
        			.width = sidebarWidth, 
        			.height = CLAY_SIZING_GROW(0) 
        		}, 
        		.padding = { 16, 16, 16, 16 }, 
        		.childGap = 16, 
        		.layoutDirection = CLAY_TOP_TO_BOTTOM 
        	}, 
        	.backgroundColor = Clay_Color({ 150, 150, 255, 255 }) 
        }) {
            CLAY(CLAY_ID("ProfilePictureOuter"), { 
            	.layout = { 
            		.sizing = { 
            			.width = CLAY_SIZING_GROW(0) 
            		}, 
            		.padding = { 8, 8, 8, 8 }, 
            		.childGap = 8, 
            		.childAlignment = { .y = CLAY_ALIGN_Y_CENTER } 
            	}, 
            	.backgroundColor = {130, 130, 255, 255}, 
            }) {
                CLAY(CLAY_ID("ProfilePicture"), { 
                	.layout = { 
                		.sizing = { 
                			.width = CLAY_SIZING_FIXED(60), 
                			.height = CLAY_SIZING_FIXED(60) 
                		},
               		},
            		.image = { .imageData = &profilePicture }, 
                });
                CLAY_TEXT(CLAY_STRING("Blah blah bla blahhasoa lalalsl"), CLAY_TEXT_CONFIG({ 
                	.textColor = Clay_Color({0, 0, 0, 255}), 
                	.fontSize = 24, 
                	.textAlignment = CLAY_TEXT_ALIGN_RIGHT 
                }));
            }

            if (Clay_Hovered() && buttonHoverId != sidebarId.id) {
                buttonHoverId = sidebarId.id;
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
            
            buttonSimple(CLAY_ID("Bweh1"), CLAY_STRING("Show Overlay"), Action::Display::DO_NOTHING);
            buttonSimple(CLAY_ID("Bweh2"), CLAY_STRING("Some Other Item"), Action::Display::DO_NOTHING);
            buttonSimple(CLAY_ID("Bweh3"), CLAY_STRING("Another Item"), Action::Display::DO_NOTHING);
            buttonSimple(CLAY_ID("Bweh4"), CLAY_STRING("More Items"), Action::Display::DO_NOTHING);

        }


        Clay_ElementId mainContentId = CLAY_ID("MainContent");
        CLAY(mainContentId, { 
        	.layout = { 
        		.sizing = { 
        			.width = CLAY_SIZING_GROW(0), 
        			.height = CLAY_SIZING_GROW(0) 
        		}, 
        		.childGap = 16, 
        		.layoutDirection = CLAY_TOP_TO_BOTTOM
        	},
    		// .backgroundColor = Clay_Color({ 200, 200, 200, 255 }),
            .image = { .imageData = &parchmentTexture },
            // pass the scrollbarData for the background image to scroll
            .userData = &scrollbarData,
        }) {
            Clay_ElementId tabId1 = CLAY_ID("Tab1");
            Clay_ElementId tabId2 = CLAY_ID("Tab2");
            Clay_ElementId tabId3 = CLAY_ID("Tab3");

            if (Clay_Hovered() && buttonHoverId != mainContentId.id) {
                buttonHoverId = mainContentId.id;
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }

            CLAY_AUTO_ID({ 
            	.layout = { 
            		.sizing = { 
            			.width = CLAY_SIZING_GROW(0) 
            		}, 
        			.padding = {8, 8, 8, 8 }, 
        			.childGap = 8, 
        			.childAlignment = { .x = CLAY_ALIGN_X_RIGHT } 
        		}, 
            	.backgroundColor =  {180, 180, 180, 255} 
        	}) {
	            buttonTab(tabId1, CLAY_STRING("Header Item 1"));
	            buttonTab(tabId2, CLAY_STRING("Header Item 2"));
	            buttonTab(tabId3, CLAY_STRING("Header Item 3"));
            }

            if (showOverlay) {                  
                CLAY(CLAY_ID("FloatingContainer"), {
                    .layout = { 
                        .sizing = { 
                            .width = CLAY_SIZING_PERCENT(0.5f), 
                            .height = CLAY_SIZING_PERCENT(0.2f) 
                            // .width = CLAY_SIZING_GROW(0), 
                            // .height = CLAY_SIZING_GROW(0) 
                        }, 
                        .padding = { 16, 16, 16, 16 }
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
                    // NOTES: CLAY_TEXT does not have .transition property, text cannot animate transition
                    // and as a result any fading on the parent leave the text unchange and looks jarring.
                    // solution is to either add .transition to each text element in Clay, or allow the parent
                    // to somehow force fade the children text nodes in it if a transition property is set.
                    CLAY_TEXT(CLAY_STRING("I'm an inline floating container."), CLAY_TEXT_CONFIG({ 
                        .textColor = Clay_Color({0,0,0,255}),
                        .fontSize = 24,
                    }));
                }
            }

            CLAY(CLAY_ID("TabContent"), {
                .layout = { 
                	.sizing = { 
                		.width = CLAY_SIZING_GROW(0),
                        .height = CLAY_SIZING_GROW(0),
                	}, 
                	.padding = { 72, 72, 72, 72 }, 
                	.childGap = 16, 
                	.layoutDirection = CLAY_TOP_TO_BOTTOM 
                },
                // .backgroundColor = {200, 200, 255, 0},
                // .image = { .imageData = &parchmentTexture },
                .clip = { 
                	.vertical = true, 
                	.childOffset = Clay_GetScrollOffset()
                },

            }) {


                if (activeTabId == tabId1.id) {     
                    CLAY(CLAY_ID("MonkFrame"), { 
                        .layout = { 
                            .sizing = { 
                                .width = CLAY_SIZING_GROW(0) 
                            }, 
                            .padding = { 8, 8, 8, 8 }, 
                            .childGap = 8, 
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } 
                        }, 
                        // .backgroundColor = {130, 130, 255, 255}, 
                    }) {
                        CLAY(CLAY_ID("MonkPicture"), { 
                            .layout = { 
                                .sizing = { 
                                    .width = CLAY_SIZING_FIXED(256), 
                                    .height = CLAY_SIZING_FIXED(256) 
                                },
                            },
                            .image = { .imageData = &monkTexture }, 
                        });
                         CLAY_TEXT(CLAY_STRING("VAE VICTIS"),
                             CLAY_TEXT_CONFIG({ 
                                .textColor = {0,0,0,200}, 
                                .fontSize = 164,
                                .letterSpacing = 0, 
                                .lineHeight = 100,
                                .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                            }));
                    }               
                     CLAY_TEXT(CLAY_STRING("Amet cursus sit amet dictum sit amet justo donec. Et malesuada fames ac turpis egestas maecenas. A lacus vestibulum sed arcu non odio euismod lacinia. Gravida neque convallis a cras. Dui nunc mattis enim ut tellus elementum sagittis vitae et. Orci sagittis eu volutpat odio facilisis mauris. Neque gravida in fermentum et sollicitudin ac orci. Ultrices dui sapien eget mi proin sed libero. Euismod quis viverra nibh cras pulvinar mattis. Diam volutpat commodo sed egestas egestas. In fermentum posuere urna nec tincidunt praesent semper. Integer eget aliquet nibh praesent tristique magna.\nId cursus metus aliquam eleifend mi in. Sed pulvinar proin gravida hendrerit lectus a. Etiam tempor orci eu lobortis elementum nibh tellus. Nullam vehicula ipsum a arcu cursus vitae. Elit scelerisque mauris pellentesque pulvinar pellentesque habitant morbi tristique senectus. Condimentum lacinia quis vel eros donec ac odio. Mattis pellentesque id nibh tortor id aliquet lectus. Turpis egestas integer eget aliquet nibh praesent tristique. Porttitor massa id neque aliquam vestibulum morbi. Mauris commodo quis imperdiet massa tincidunt nunc pulvinar sapien et. Nunc scelerisque viverra mauris in aliquam sem fringilla. Suspendisse ultrices gravida dictum fusce ut placerat orci nulla.\nLacus laoreet non curabitur gravida arcu ac tortor dignissim. Urna nec tincidunt praesent semper feugiat nibh sed pulvinar. Tristique senectus et netus et malesuada fames ac. Nunc aliquet bibendum enim facilisis gravida. Egestas maecenas pharetra convallis posuere morbi leo urna molestie. Sapien nec sagittis aliquam malesuada bibendum arcu vitae elementum curabitur. Ac turpis egestas maecenas pharetra convallis posuere morbi leo urna. Viverra vitae congue eu consequat. Aliquet enim tortor at auctor urna. Ornare massa eget egestas purus viverra accumsan in nisl nisi. Elit pellentesque habitant morbi tristique senectus et netus et malesuada.\nSuspendisse ultrices gravida dictum fusce ut placerat orci nulla pellentesque. Lobortis feugiat vivamus at augue eget arcu. Vitae justo eget magna fermentum iaculis eu. Gravida rutrum quisque non tellus orci. Ipsum faucibus vitae aliquet nec. Nullam non nisi est sit amet. Nunc consequat interdum varius sit amet mattis vulputate enim. Sem fringilla ut morbi tincidunt augue interdum. Vitae purus faucibus ornare suspendisse. Massa tincidunt nunc pulvinar sapien et. Fringilla ut morbi tincidunt augue interdum velit euismod in. Donec massa sapien faucibus et. Est placerat in egestas erat imperdiet. Gravida rutrum quisque non tellus. Morbi non arcu risus quis varius quam quisque id diam. Habitant morbi tristique senectus et netus et malesuada fames ac. Eget lorem dolor sed viverra.\nOrnare massa eget egestas purus viverra. Varius vel pharetra vel turpis nunc eget lorem. Consectetur purus ut faucibus pulvinar elementum. Placerat in egestas erat imperdiet sed euismod nisi. Interdum velit euismod in pellentesque massa placerat duis ultricies lacus. Aliquam nulla facilisi cras fermentum odio eu. Est pellentesque elit ullamcorper dignissim cras tincidunt. Nunc sed id semper risus in hendrerit gravida rutrum. A pellentesque sit amet porttitor eget dolor morbi. Pellentesque habitant morbi tristique senectus et netus et malesuada fames. Nisl nunc mi ipsum faucibus vitae aliquet nec ullamcorper. Sed id semper risus in hendrerit gravida. Tincidunt praesent semper feugiat nibh. Aliquet lectus proin nibh nisl condimentum id venenatis a. Enim sit amet venenatis urna cursus eget. In egestas erat imperdiet sed euismod nisi porta lorem mollis. Lacinia quis vel eros donec ac odio tempor orci. Donec pretium vulputate sapien nec sagittis aliquam malesuada bibendum arcu. Erat pellentesque adipiscing commodo elit at.\nEgestas sed sed risus pretium quam vulputate. Vitae congue mauris rhoncus aenean vel elit scelerisque mauris pellentesque. Aliquam malesuada bibendum arcu vitae elementum. Congue mauris rhoncus aenean vel elit scelerisque mauris. Pellentesque dignissim enim sit amet venenatis urna cursus. Et malesuada fames ac turpis egestas sed tempus urna. Vel fringilla est ullamcorper eget nulla facilisi etiam dignissim. Nibh cras pulvinar mattis nunc sed blandit libero. Fringilla est ullamcorper eget nulla facilisi etiam dignissim. Aenean euismod elementum nisi quis eleifend quam adipiscing vitae proin. Mauris pharetra et ultrices neque ornare aenean euismod elementum. Ornare quam viverra orci sagittis eu. Odio ut sem nulla pharetra diam sit amet nisl suscipit. Ornare lectus sit amet est. Ullamcorper sit amet risus nullam eget. Tincidunt lobortis feugiat vivamus at augue eget arcu dictum.\nUrna nec tincidunt praesent semper feugiat nibh. Ut venenatis tellus in metus vulputate eu scelerisque felis. Cursus risus at ultrices mi tempus. In pellentesque massa placerat duis ultricies lacus sed turpis. Platea dictumst quisque sagittis purus. Cras adipiscing enim eu turpis egestas. Egestas sed tempus urna et pharetra pharetra. Netus et malesuada fames ac turpis egestas integer eget aliquet. Ac turpis egestas sed tempus. Sed lectus vestibulum mattis ullamcorper velit sed. Ante metus dictum at tempor commodo ullamcorper a. Augue neque gravida in fermentum et sollicitudin ac. Praesent semper feugiat nibh sed pulvinar proin gravida. Metus aliquam eleifend mi in nulla posuere sollicitudin aliquam ultrices. Neque gravida in fermentum et sollicitudin ac orci phasellus egestas.\nRidiculus mus mauris vitae ultricies. Morbi quis commodo odio aenean. Duis ultricies lacus sed turpis. Non pulvinar neque laoreet suspendisse interdum consectetur. Scelerisque eleifend donec pretium vulputate sapien nec sagittis aliquam. Volutpat est velit egestas dui id ornare arcu odio ut. Viverra tellus in hac habitasse platea dictumst vestibulum rhoncus est. Vestibulum lectus mauris ultrices eros. Sed blandit libero volutpat sed cras ornare. Id leo in vitae turpis massa sed elementum tempus. Gravida dictum fusce ut placerat orci nulla pellentesque. Pretium quam vulputate dignissim suspendisse in. Nisl suscipit adipiscing bibendum est ultricies integer quis auctor. Risus viverra adipiscing at in tellus. Turpis nunc eget lorem dolor sed viverra ipsum. Senectus et netus et malesuada fames ac. "),
                         CLAY_TEXT_CONFIG({ 
                            .textColor = {0,0,0,255}, 
                            .fontSize = 48,
                            .letterSpacing = 0, 
                            .lineHeight = 30,
                            .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                        }));
                } else if (activeTabId == tabId2.id) {  
                    CLAY_TEXT(CLAY_STRING("Faucibus purus in massa tempor nec. Nec ullamcorper sit amet risus nullam eget felis eget nunc. Diam vulputate ut pharetra sit amet aliquam id diam. Lacus suspendisse faucibus interdum posuere lorem. A diam sollicitudin tempor id. Amet massa vitae tortor condimentum lacinia. Aliquet nibh praesent tristique magna."),
                        CLAY_TEXT_CONFIG({ 
                            .textColor = {0,0,0,255}, 
                            .fontSize = 48,
                            .letterSpacing = 0, 
                            .lineHeight = 30,
                            .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                        }));

                    CLAY_TEXT(CLAY_STRING("Suspendisse in est ante in nibh. Amet venenatis urna cursus eget nunc scelerisque viverra. Elementum sagittis vitae et leo duis ut diam quam nulla. Enim nulla aliquet porttitor lacus. Pellentesque habitant morbi tristique senectus et. Facilisi nullam vehicula ipsum a arcu cursus vitae.\nSem fringilla ut morbi tincidunt. Euismod quis viverra nibh cras pulvinar mattis nunc sed. Velit sed ullamcorper morbi tincidunt ornare massa. Varius quam quisque id diam vel quam. Nulla pellentesque dignissim enim sit amet venenatis. Enim lobortis scelerisque fermentum dui faucibus in. Pretium viverra suspendisse potenti nullam ac tortor vitae. Lectus vestibulum mattis ullamcorper velit sed. Eget mauris pharetra et ultrices neque ornare aenean euismod elementum. Habitant morbi tristique senectus et. Integer vitae justo eget magna fermentum iaculis eu. Semper quis lectus nulla at volutpat diam. Enim praesent elementum facilisis leo. Massa vitae tortor condimentum lacinia quis vel."),
                        CLAY_TEXT_CONFIG({ 
                            .textColor = {0,0,0,255}, 
                            .fontSize = 48,
                            .letterSpacing = 0, 
                            .lineHeight = 30,
                            .textAlignment = CLAY_TEXT_ALIGN_LEFT 
                        }));
                
                } if (activeTabId == tabId3.id) {  

                     CLAY(CLAY_ID("Photos"), { 
                        .layout = { 
                            .sizing = { 
                                .width = CLAY_SIZING_GROW(0) 
                            }, 
                            .padding = {16, 16, 16, 16}, 
                            .childGap = 16, 
                            .childAlignment = { 
                                .x = CLAY_ALIGN_X_CENTER, 
                                .y = CLAY_ALIGN_Y_CENTER 
                            } 
                        }, 
                        .backgroundColor = {180, 180, 220, 255},
                        // .image = { .imageData = &parchmentTexture },
                     }) {
                         CLAY(CLAY_ID("Picture2"), { 
                            .layout = { 
                                .sizing = { 
                                    .width = CLAY_SIZING_FIXED(120) 
                                }
                            }, 
                            .overlayColor = { 100, 0, 0, 140 },
                            .aspectRatio = { 0.5f },
                            .image = { .imageData = &profilePicture },
                         });
                         CLAY(CLAY_ID("Picture1"), { 
                            .layout = { 
                                .padding = {8, 8, 8, 8}, 
                                .childAlignment = { 
                                    .x = CLAY_ALIGN_X_CENTER 
                                }, 
                                .layoutDirection = CLAY_TOP_TO_BOTTOM 
                            }, 
                            .backgroundColor = {170, 170, 220, 255} 
                         }) {
                             CLAY(CLAY_ID("ProfilePicture2"), { 
                                .layout = { 
                                    .sizing = { 
                                        .width = CLAY_SIZING_FIXED(60), 
                                        .height = CLAY_SIZING_FIXED(60) 
                                    }
                                },
                                .image = { .imageData = &profilePicture }, 
                             });
                             CLAY_TEXT(CLAY_STRING("Image caption below"), CLAY_TEXT_CONFIG({ .textColor = {0,0,0,255}, .fontSize = 24 }));
                         }
                         CLAY(CLAY_ID("Picture3"), { 
                            .layout = { 
                                .sizing = { 
                                    .width = CLAY_SIZING_FIXED(120) 
                                }
                            }, 
                            .aspectRatio = { 2.0f },
                            .image = { .imageData = &profilePicture }, 
                         });
                     }

                } else if (activeTabId == 0) {
                    activeTabId = tabId1.id;
                }


            } // main content

	        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("TabContent")));
	        if (scrollData.found && scrollData.scrollContainerDimensions.height < scrollData.contentDimensions.height) {
	            CLAY(CLAY_ID("ScrollBar"), {
	                .floating = {
	                    .offset = { 
	                    	.y = -(scrollData.scrollPosition->y / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height 
	                    },
	                    .parentId = Clay_GetElementId(CLAY_STRING("TabContent")).id,
	                    .zIndex = 1,
	                    .attachPoints = { 
	                    	.element = CLAY_ATTACH_POINT_RIGHT_TOP, 
	                    	.parent = CLAY_ATTACH_POINT_RIGHT_TOP 
	                    },
	                    .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
	                }
	            }) {
	                CLAY(CLAY_ID("ScrollBarButton"), {
	                    .layout = { 
	                    	.sizing = { 
	                    		CLAY_SIZING_FIXED(12), 
	                    		CLAY_SIZING_FIXED((scrollData.scrollContainerDimensions.height / scrollData.contentDimensions.height) * scrollData.scrollContainerDimensions.height) 
	                    	}
	                    },
	                    .backgroundColor = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar"))) ? Clay_Color({100, 100, 140, 150}) : Clay_Color({120, 120, 160, 150}),
	                    .cornerRadius = CLAY_CORNER_RADIUS(6),
	                });
	            }
	        }
        } // right panel


    } // container

}

void Display::handleError(Clay_ErrorData errorData) {
    TraceLog(LOG_INFO, "%s", errorData.errorText.chars);
    Display* self = static_cast<Display*>(errorData.userData);

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
    Clay_Initialize(self->arena, Clay_Dimensions({ static_cast<float>(self->screen.width()), static_cast<float>(self->screen.height()) }), Clay_ErrorHandler({ .errorHandlerFunction = self->handleError, .userData = self }));
}

void Display::onScreenResize(int width, int height) {
    if (width < 720) {
        sidebarWidth = CLAY_SIZING_FIXED(150);
    } else {
        sidebarWidth = CLAY_SIZING_PERCENT(0.2f);
    }

	Clay_SetLayoutDimensions(Clay_Dimensions({ static_cast<float>(width), static_cast<float>(height) }));
}

void Display::unload(){
    if(temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer_len = 0;

    UnloadShader(overlayShader);
    UnloadFont(fonts[0]);
    UnloadFont(fonts[1]);

    free(arena.memory);

    UnloadTexture(parchmentTexture);
    UnloadTexture(profilePicture);
    UnloadTexture(monkTexture);
}
