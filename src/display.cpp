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

    fonts[0] = LoadFontEx(PATH_ASSET("Roboto-Regular.ttf"), 48, 0, 400);
    SetTextureFilter(fonts[0].texture, TEXTURE_FILTER_BILINEAR);
    fonts[1] = LoadFontEx(PATH_ASSET("Roboto-Regular.ttf"), 32, 0, 400);
    SetTextureFilter(fonts[1].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    initOverlay();
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
                DrawTexturePro(
                    imageTexture,
                    Rectangle({ 0, 0, static_cast<float>(imageTexture.width), static_cast<float>(imageTexture.height) }),
                    Rectangle({boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height}),
                    Vector2({}),
                    0,
                    CLAY_COLOR_TO_RAYLIB_COLOR(tintColor));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                BeginScissorMode((int)roundf(boundingBox.x), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width), (int)roundf(boundingBox.height));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                EndScissorMode();
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START: {
                setColorOverlay(CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->renderData.overlayColor.color));
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END: {
                disableColorOverlay();
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

void Display::setColorOverlay(Color color) const {
    overlayEnabled = true;
    float colorFloat[4] = {
        (float)color.r/255.0f,
        (float)color.g/255.0f,
        (float)color.b/255.0f,
        (float)color.a/255.0f,
    };

    SetShaderValue(overlayShader, colorLoc, colorFloat, SHADER_UNIFORM_VEC4);
    BeginShaderMode(overlayShader);
}

void Display::disableColorOverlay() const {
    if (overlayEnabled) {
        EndShaderMode();
        overlayEnabled = false;
    }
}

void Display::button(Clay_String buttonText) {
	// Clay_Color bgColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE); 
    // Red box button with 8px of padding
    CLAY_AUTO_ID({ 
    	.layout = { 
    		.padding = CLAY_PADDING_ALL(8) 
    	}, 
    	.backgroundColor = Clay_Hovered() ? RAYLIB_COLOR_TO_CLAY_COLOR(GREEN) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE) 
    }) {
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({ .textColor = {0,0,0,255}, .fontSize = 24 }));
    }
}

void Display::update(){

    // Clay_BeginLayout();
     
    // Root container
    CLAY(CLAY_ID("MainContent"), {
        .layout = { 
            .sizing = { CLAY_SIZING_GROW(1), CLAY_SIZING_GROW(1) },
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 16 
        },
        .backgroundColor = {250, 250, 255, 0}
    }) {
        // A nested child
        CLAY(CLAY_ID("Sidebar"), {
            .layout = {
                .sizing = { 
                    // .width = CLAY_SIZING_FIXED(screen.width()*0.3f), 
                    // .height = CLAY_SIZING_FIXED(screen.height()*0.8f) 
                    .width = CLAY_SIZING_FIXED(300), 
                    .height = CLAY_SIZING_GROW(1) 
                },
            	.layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
            .backgroundColor = { 200, 200, 200, 255 }
        }) {
    		// Clay_OnHover([](Clay_ElementId id, Clay_PointerData ptr, void* userData){ printf("Button hover"); }, 0);
            CLAY_TEXT(CLAY_STRING("I'm an inline floating container."), CLAY_TEXT_CONFIG({ .textColor = {0,0,0,255}, .fontSize = 24 }));
        	button(CLAY_STRING("HAHAHAHA"));
        	button(CLAY_STRING("BWEH"));
        }
    }
     
    // Clay_RenderCommandArray renderCommands = Clay_EndLayout(GetFrameTime());

    // return renderCommands;
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
	Clay_SetLayoutDimensions(Clay_Dimensions({ static_cast<float>(width), static_cast<float>(height) }));
    // Clay_Context* context = Clay_GetCurrentContext();
    // if (context != nullptr) {
    // 	context->layoutDimensions = Clay_Dimensions({ static_cast<float>(width), static_cast<float>(height) });
    // }
    // Clay_Initialize(arena, Clay_Dimensions({ static_cast<float>(width), static_cast<float>(height) }), Clay_ErrorHandler({ .errorHandlerFunction = handleError, .userData = this }));
}

void Display::unload(){
    if(temp_render_buffer) free(temp_render_buffer);
    temp_render_buffer_len = 0;

    UnloadShader(overlayShader);
    UnloadFont(fonts[0]);
    UnloadFont(fonts[1]);

    free(arena.memory);
}
