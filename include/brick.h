/*

# BRICK UI 
v0.1

## USAGE
Define BRICK_IMPLEMENTATION in exactly ONE file only.
Then include brick.h in the line after, and/or other files.

```C/C++
#define BRICK_IMPLEMENTATION
#include "brick.h"
```

DO NOT define CLAY_IMPLEMENTATION. Brick owns Clay, but files can include clay.h for types or other utilities.

## DESCRIPTION
Clay is immediate mode. Every frame, the UI is declared from scratch, laid out, rendered, and discarded. Brick is the stateful layer on top. It remembers which buttons exist, what their IDs are, and what their interaction state was last frame. It turns Clay's stateless per-frame declarations into a system where the user can say "did button 47 get clicked?" in a natural way.

OBJ:
1. add basic UI state, i.e. button hover or not, button is toggled, panel is visible or hidden
2. separate content from layout, i.e. declar button and labels in one place, use them in the layout later
3. uses 'embeded scope constructs' like Raylib to match this C style of API
4. provide sensible defaults where possible, windows grow, have some padding, etc
5. provide a global config that overrides defaults, change colors, adjust padding
6. add responsive reactive behavior to the window, i.e. resize on event window resizing and adjust layout dynamically
7. provide an easy way to localize, i.e. through Brick_TextEx or TextPro or a Brick_LocalizedText, that would stand in for normal strings and can be globally configured

*/

#ifdef BRICK_IMPLEMENTATION
#define CLAY_IMPLEMENTATION
#endif
#include "clay.h"

#ifndef BRICK_HEADER
#define BRICK_HEADER

// Default Settings 
// ---------------------------------------------------------------

#define BRICK_MAX_BUTTONS 256
#define BRICK_MAX_BUTTON_GROUP_SIZE 16
#define BRICK_MAX_BUTTON_GROUPS 16
#define BRICK_MAX_SCROLLBOXES 32
#define BRICK_MAX_ELEMENTS (BRICK_MAX_BUTTONS + BRICK_MAX_SCROLLBOXES)

// Default Styles 
// ---------------------------------------------------------------
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

#define STYLE_TEXT_DEFAULT CLAY_TEXT_CONFIG({ .textColor = SURFACE_COLOR_FG, .fontId = 1, .fontSize = 24 })

#define STYLE_TEXT_CENTERED CLAY_TEXT_CONFIG({ .textColor = Clay_Color({ 200, 200, 200, 255 }), .fontSize = 24, .textAlignment = CLAY_TEXT_ALIGN_CENTER })

// Public Types 
// ---------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float width;
    float height;
    int32_t hoveredId;
    int32_t lastHoveredId;
} Brick_Window;

typedef struct {
    float x;
    float y;
    bool pressed;
} Brick_PointerData;

typedef CLAY_PACKED_ENUM {
    BRICK_ELEMENT_TYPE_BUTTON,
    BRICK_ELEMENT_TYPE_PANEL,
    BRICK_ELEMENT_TYPE_BUTTON_GROUP,
} Brick_ElementType;

typedef struct Brick_ElementId {
    int32_t index;
    Brick_ElementType type;
} Brick_ElementId;

typedef struct {
    Clay_ElementId clayId;
    Clay_String label;
    Brick_ElementId id;
    bool hovered;
    bool cleared;
    bool clicked;
    bool pressed;
    bool released;
    bool toggled;
} Brick_Button;

typedef struct {
    int32_t length;
    int32_t ids[BRICK_MAX_BUTTON_GROUP_SIZE];
} Brick_ElementGroup;

#define BRICK_MAX_EVENT_TYPES 10
// Different event types triggered by element interactions
typedef CLAY_PACKED_ENUM {
    // This event should be skipped.
    BRICK_EVENT_NONE,
    // Triggers when the element is hovered (first frame only)
    BRICK_EVENT_HOVER,
    // Triggers when the element is being hovered
    BRICK_EVENT_HOVERING,
    // Triggers on the frame hovering stops
    BRICK_EVENT_CLEAR,
    // Triggers when the element is pressed with the primary input (first frame only)
    BRICK_EVENT_PRESS,
    // Triggers while the element is being pressed on across frames
    BRICK_EVENT_PRESSING,
    // Triggers on the exact frame (or delayed by one) the press was released
    BRICK_EVENT_RELEASE,
} Brick_EventType;

typedef struct Brick_Event {
    // internal brick id
    int32_t id;
    // event state for the element
    Brick_EventType eventType;
} Brick_Event;

// events array container
typedef struct Brick_EventArray {
    int32_t length;
    Brick_Event* data;
} Brick_EventArray;

#ifdef __cplusplus
}
#endif

// Function Forward Declarations ---------------------------------
// Public API functions ------------------------------------------

Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index);

#endif /* BRICK_HEADER */

// IMPLEMENTATION 
// ---------------------------------------------------------------

#ifdef BRICK_IMPLEMENTATION
#undef BRICK_IMPLEMENTATION

Brick_Event Brick_Event_DEFAULT = CLAY__DEFAULT_STRUCT;
Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_Event_DEFAULT;
}    

typedef struct Brick_ButtonArray {
    int32_t length;
    Brick_Button* data;
} Brick_ButtonArray;

Brick_Button Brick_Button_DEFAULT = CLAY__DEFAULT_STRUCT;
Brick_Button* Brick_ButtonArray_Get(Brick_ButtonArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_Button_DEFAULT;
}    

typedef struct Brick_ButtonGroupArray {
    int32_t length;
    Brick_ElementGroup* data;
} Brick_ButtonGroupArray;

Brick_ElementGroup Brick_ElementGroup_DEFAULT = CLAY__DEFAULT_STRUCT;
Brick_ElementGroup* Brick_ButtonGroupArray_Get(Brick_ButtonGroupArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_ElementGroup_DEFAULT;
}

typedef struct Brick_Elements {
    int32_t total_count;
    Brick_ButtonArray buttons;
    Brick_ButtonGroupArray buttonGroups;
} Brick_Elements;

// Global Context
// --------------------------

// Clay context
static Clay_Arena g_clay_arena = CLAY__DEFAULT_STRUCT;

// window state also holds pointer state
static Brick_Window g_window = CLAY__DEFAULT_STRUCT;

// element state arrays
static Brick_Button g_buttons[BRICK_MAX_BUTTONS];
static Brick_ElementGroup g_button_groups[BRICK_MAX_BUTTON_GROUPS];

static Brick_Elements g_elements = {
    .total_count = 0,
    .buttons = {
        .length = 0,
        .data = g_buttons
    },
    .buttonGroups = {
        .length = 0,
        .data = g_button_groups
    },
};

static bool g_is_events_snapshot_dirty = false;
static bool g_events_snapshot[BRICK_MAX_EVENT_TYPES] = CLAY__DEFAULT_STRUCT;
// event array passed back to user to handle events
static Brick_Event g_events[BRICK_MAX_ELEMENTS];

// Button internals
// ---------------------------------------------------------------

void Brick_OnButtonHover(int32_t idx, bool isHovering) {
    // Sets the following flags on the button:
    // hovered: the pointer is over the button (multiple frames)
    // cleared: the pointer has just stopped hovering (1 frame)

    // NOTE: gets called on every frame with every button
    Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, idx);
    
    // The button indexes are saved on the global context (g_window)
    // hoveredId: the current button being hovered
    // lastHoveredId: the last button that was hovered (after hovering on a new one) 
    if (isHovering) {
        // entering hover on button
        if (g_window.hoveredId != idx && g_window.lastHoveredId != idx) {
            g_window.hoveredId = idx;
            button->hovered = true;
        // one frame after entering hover
        } else if (g_window.hoveredId == idx && g_window.lastHoveredId != idx) {
            // propagate the cache to the last hover state
            g_window.lastHoveredId = idx;
        } 
    } else {
        // exiting hover
        if (g_window.hoveredId == idx) {
            g_window.hoveredId = 0;
            button->hovered = false;
            button->cleared = true;
        // one frame after exiting hover. Note: checking both last hover state, 
        // and the cleared flag for cases when pointer is moving really fast
        } else if (g_window.lastHoveredId == idx || button->cleared) {
            g_window.lastHoveredId = 0;
            button->cleared = false;
        }
    }
}

void Brick_HandleClayHover(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    
    for (int32_t i = 1; i < g_elements.buttons.length; i++) {
        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, i);
        
        if(button->clayId.id == elementId.id) {
            switch(pointerData.state) {
            case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
                // printf("CLICK %d\n", elementId.id);
                button->clicked = true;
                button->toggled = !button->toggled;
                break;
            case CLAY_POINTER_DATA_PRESSED:
                button->clicked = false;
                button->pressed = true;
                break;
            case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
                button->clicked = false;
                button->pressed = false;
                button->released = true;
                break;
            case CLAY_POINTER_DATA_RELEASED:
                // NOTE: This is just the same as hover, Clay triggers this if pointer 
                // is on the button not pressing, and after pressing
                break;
            default: break;
            }
            break;
        }
    }
}


// Public API
// ----------------------------------

void Brick_HandleError(Clay_ErrorData errorData);

Brick_ElementId Brick_CreateElementId(int32_t index, Brick_ElementType type);
Brick_ElementId Brick_CreateButton(const char* label);
Brick_ElementId Brick_GroupButtons(const Brick_ElementId* buttonIds, int32_t groupSize);

// initializes Clay first, then Brick
void Brick_Initialize(float width, float height, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void *fontData), void *fontData) {
    printf("Initializing Brick\n");
    // cache window dimensions
    g_window.width = width;
    g_window.height = height;

    // 1. Query minimum memory required for default element limits
    uint64_t memorySize = Clay_MinMemorySize();
    // 2. Allocate memory (malloc, stack, or custom allocator)
    void* memory = malloc(memorySize);
    // 3. Create arena [clay.h:2150-2158]
    g_clay_arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    // 4. Initialize Clay [clay.h:2186-2188]
    Clay_Initialize(g_clay_arena, Clay_Dimensions({ width, height }), Clay_ErrorHandler({ .errorHandlerFunction = Brick_HandleError, .userData = nullptr }));
    // 5. Set the MeasureText function along with pointer to fonts
    Clay_SetMeasureTextFunction(measureTextFunction, fontData);

    // seed button array and button group array at index 0 as unit values
    Brick_CreateButton("BRICK");
    // bypassing Brick_GroupButtons that checks 0 as invalid
    g_elements.buttonGroups.data[0] = Brick_ElementGroup_DEFAULT;
    g_elements.buttonGroups.length++;
}

// simple wrapper around Clay_BeginLayout
void Brick_BeginLayout(void) {
    Clay_BeginLayout();
}

// simple wrapper around Clay_EndLayout which returns render commands
Clay_RenderCommandArray Brick_EndLayout(float deltaTime) {
    return Clay_EndLayout(deltaTime);
}

// cleans up Brick then Clay
void Brick_Destroy(void) {
    if(g_clay_arena.memory) free(g_clay_arena.memory);
}

Brick_ElementId Brick_CreateElementId(int32_t index, Brick_ElementType type) {
    Brick_ElementId id = { index, type };
    return id;
}

// Brick elements Add<Element> initializes the element and is to be called once
Brick_ElementId Brick_CreateButton(const char* label) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(label), 
        .chars = label 
    };

    int32_t index = g_elements.buttons.length;
    Brick_ElementId buttonId = {
        .index = index,
        .type = BRICK_ELEMENT_TYPE_BUTTON,
    };

    Brick_Button new_button = {
        .clayId = CLAY_SID(clayString),
        .label = clayString,
        .id = buttonId,
        .hovered = false,
        .cleared = false,
        .clicked = false,
        .pressed = false,
        .released = false,
        .toggled = false,
    };

    g_buttons[index] = new_button;
    g_elements.buttons.length++;
    g_elements.total_count++;

    return buttonId;
}

Brick_ElementId Brick_GroupButtons(const Brick_ElementId* buttonIds, int32_t groupSize) {

    Brick_ElementGroup group = CLAY__DEFAULT_STRUCT;

    for (int32_t i = 0; i < groupSize; i++) {
        if (buttonIds[i].type != BRICK_ELEMENT_TYPE_BUTTON) {
            // TODO: exit or handle error instead of return 0 here (0 is valid group!)
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
        }

        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonIds[i].index);

        if (button->id.index == 0) {
            // TODO: exit or handle error instead of return 0 here (0 is valid group!)
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
        }

        group.ids[i] = button->id.index;
        group.length++;
    }

    int32_t index = g_elements.buttonGroups.length;
    g_elements.buttonGroups.data[index] = group;
    g_elements.buttonGroups.length++;

    return Brick_CreateElementId(index, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
}

// Global pointer hover check on any button. This is meant to be used in PollEvents.
// WARN: using this function by itself can be a race condition with the button HoverHandler
bool Brick_PointerJustHovered() {
    return g_window.hoveredId != 0 && g_window.lastHoveredId != g_window.hoveredId;
}
// Global pointer hover clear check on any button. This is meant to be used in PollEvents.
// WARN: using this function by itself can be a race condition with the button HoverHandler
bool Brick_PointerJustCleared() {
    return g_window.hoveredId == 0 && g_window.lastHoveredId != 0;
}

// Brick only function that will handle any potential updates of elements per frame
Brick_EventArray Brick_PollEvents(Brick_PointerData pointerData) {

    Clay_SetPointerState(Clay_Vector2({ .x = pointerData.x, .y = pointerData.y }), pointerData.pressed);
    
    Brick_EventArray events = {
        .length = 0,
        .data = g_events
    };

    if (g_is_events_snapshot_dirty) {
        for (int32_t i = 1; i < BRICK_MAX_EVENT_TYPES; i++) {
            g_events_snapshot[i] = false;
        }
    }

    // Button Events ------------------------------------
    // skip unit button at index 0
    for (int32_t i = 1; i < g_elements.total_count; i++) {
        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, i);

        if(button->clicked && !button->pressed) {
            // prevents event from firing after button is
            // not rendered, i.e. clicking to change panels
            button->clicked = false;
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_PRESS
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_PRESS] = true;
        } 
        else if (button->pressed) { 
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_PRESSING
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_PRESSING] = true;
        }
        else if(button->released) {
            // prevents from firing after button is
            // blocked or not rendered, i.e. showing a popup window
            button->released = false;
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_RELEASE
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_RELEASE] = true;
        }
        else if(button->hovered) {
            Brick_EventType eventType = Brick_PointerJustHovered() ? BRICK_EVENT_HOVER : BRICK_EVENT_HOVERING;
            g_events[events.length] = {
                .id = i,
                .eventType = eventType
            };
            events.length++;

            g_events_snapshot[eventType] = true;
        }
        else if(button->cleared) {
            g_events[events.length] = {
                .id = i,
                .eventType = BRICK_EVENT_CLEAR
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_CLEAR] = true;
        }
    }

    if (events.length) g_is_events_snapshot_dirty = true;

    return events;
}

bool Brick_OnEventTriggered(Brick_EventType eventType) {
    // TODO: add some error handling
    if (eventType > BRICK_MAX_EVENT_TYPES) return false;
    
    return g_events_snapshot[eventType];
}

// Layout<element> is to be called within CLAY macros which are also encapsulated in other Brick elements
void Brick_LayoutButton(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);

    CLAY(button->clayId, {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(8),
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
        }, 
        // Clay_Hovered only works inside the paramaters or Clay declaration body
        .backgroundColor = Clay_Hovered() ? Clay_Color({ 140, 140, 140, 255 }) : Clay_Color({ 216, 238, 10, 255 }),
        .border = { 
            .color = Clay_Color({ 140, 140, 140, 255 }), 
            .width = CLAY_BORDER_OUTSIDE(1) 
        }
        // .transition = {
        //     .handler = Clay_EaseOut,
        //     .duration = 0.3f,
        //     .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
        //     .enter = { .setInitialState = FadeSlide },
        //     // .exit = { .setFinalState = FadeSlide },
        // }
    }) {
        Brick_OnButtonHover(button->id.index, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHover, nullptr);
        CLAY_TEXT(button->label, STYLE_TEXT_CENTERED);
    }
}

void Brick_LayoutButtonGroup(Brick_ElementId groupId) {
    // TODO: add error handling
    if (groupId.type != BRICK_ELEMENT_TYPE_BUTTON_GROUP) return;

    const Brick_ElementGroup* buttonGroup = Brick_ButtonGroupArray_Get(&g_elements.buttonGroups, groupId.index);

    for (int32_t i = 0; i < buttonGroup->length; i++) {
        // TODO: create another internal interface to skip constructing brick_elementID
        Brick_LayoutButton(Brick_CreateElementId(buttonGroup->ids[i], BRICK_ELEMENT_TYPE_BUTTON ));
    }
}

// BeginLayout<element> and EndLayout<element> are the opening and close functions for container elements
void Brick_BeginLayoutPanel(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(0.5f),
                .height = CLAY_SIZING_PERCENT(0.5f),
            },
            .padding = CLAY_PADDING_ALL(32), 
            .childGap = 12, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = Clay_Color({ 140, 140, 140, 255 }),
        .floating = { 
            .offset = {0, 0}, 
            .zIndex = 1, 
            .attachPoints = { 
                CLAY_ATTACH_POINT_CENTER_CENTER, 
                CLAY_ATTACH_POINT_CENTER_CENTER 
            }, 
            .attachTo = CLAY_ATTACH_TO_PARENT 
        },
    });
}

void Brick_EndLayoutPanel(void) {
    Clay__CloseElement();
}

void Brick_BeginLayoutTogglePanel(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);

    if (button->toggled) {
        Brick_BeginLayoutPanel();
    }
}

void Brick_EndLayoutTogglePanel(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON) return;

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);
    
    if (button->toggled) {
        Clay__CloseElement();
    }
}

void Brick_HandleError(Clay_ErrorData errorData) {

    switch(errorData.errorType) {

        case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_FUNCTION_NOT_PROVIDED:
            printf("CLAY ERROR: Text Measurement Function not provided.");
            break;
        // Clay attempted to allocate its internal data structures but ran out of space.
        // The arena passed to Clay_Initialize was created with a capacity smaller than that required by Clay_MinMemorySize().
        case CLAY_ERROR_TYPE_ARENA_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Arena capacity exceeded.");
            break;
        // Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxElementCount().
        case CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Elements capacity exceeded.");
            break;
        // Clay ran out of capacity in its internal array for storing elements. This limit can be increased with Clay_SetMaxMeasureTextCacheWordCount().
        case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Text measurement capacity exceeded.");
            break;
        // Two elements were declared with exactly the same ID within one layout.
        case CLAY_ERROR_TYPE_DUPLICATE_ID:
            printf("CLAY ERROR: Duplicate ID.");
            break;
        // A floating element was declared using CLAY_ATTACH_TO_ELEMENT_ID and either an invalid .parentId was provided or no element with the provided .parentId was found.
        case CLAY_ERROR_TYPE_FLOATING_CONTAINER_PARENT_NOT_FOUND:
            printf("CLAY ERROR: Floating container parent not found.");
            break;
        // An element was declared that using CLAY_SIZING_PERCENT but the percentage value was over 1. Percentage values are expected to be in the 0-1 range.
        case CLAY_ERROR_TYPE_PERCENTAGE_OVER_1:
            printf("CLAY ERROR: Percentage over 1.");
            break;
        // Clay encountered an internal error. It would be wonderful if you could report this so we can fix it!
        case CLAY_ERROR_TYPE_INTERNAL_ERROR:
            printf("CLAY ERROR: Internal error.");
            break;
        // Clay__OpenElement was called more times than Clay__CloseElement, so there were still remaining open elements when the layout ended.
        case CLAY_ERROR_TYPE_UNBALANCED_OPEN_CLOSE:
            printf("CLAY ERROR: Unbalanced open-close.");
            break;
        case CLAY_ERROR_TYPE_HASH_MAP_CAPACITY_EXCEEDED:
            printf("CLAY ERROR: Hash map capacity exceeded.");
            break;
        default: break;
    }
    
    // NOTE: this handles Clay exceed element count
    // TODO: decide to initialize Clay only or Brick & Clay, and how this would work
    // with Brick element max count
    // if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
    //     Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
    //     Brick_Initialize(g_window.width, g_window.height);
    // } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
    //     Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    //     Brick_Initialize(g_window.width, g_window.height);
    // }
}

#endif /* BRICK_IMPLEMENTATION */

/*
LICENSE
zlib/libpng license

Copyright (c) 2026 Alan Vincenzi

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
    be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/
