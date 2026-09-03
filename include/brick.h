/* v0.1
+-------------+
|    BRICK    |
+-------------+ 
Clay Extension

USAGE SUMMARY
This is a very high level overview.
For more details see docs: 

1. Import library
DO NOT define CLAY_IMPLEMENTATION
----------------------------------
- #define BRICK_IMPLEMENTATION
- #include "brick.h"

2. Initialize 
DO NOT call these in a loop!
----------------------------------
- Brick_Initialize 
- Create Elements and save Ids

3. Update
Call update before layout
----------------------------------
- Brick_Update -> events
- Loop and handle events

4. Layout
DO NOT forget to close containers!
----------------------------------
- Brick_BeginLayout
- Brick_Begin<Container>
- Brick_Layout<Element>
- Brick_End<Container>
- Brick_EndLayout -> render-commands 

5. Render
----------------------------------
- Loop and render render-commands

Define BRICK_IMPLEMENTATION in exactly ONE file only.
Then include brick.h in the line after, and/or other files.
```C/C++
#define BRICK_IMPLEMENTATION
#include "brick.h"
```
DO NOT define CLAY_IMPLEMENTATION. Brick owns Clay, but files can include clay.h for types or other utilities.


DESCRIPTION
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

#if defined(__cplusplus)
    #define PLEX(type) type
#else
    #define PLEX(type) (type)
#endif

// ========================================================================================
//                                      DEFAULTS
// ========================================================================================

// Settings 
// ---------------------------------------------------------------
#define BRICK_MAX_BUTTONS 256
#define BRICK_MAX_BUTTON_GROUP_SIZE 16
#define BRICK_MAX_BUTTON_GROUPS 16
#define BRICK_MAX_SCROLLBOXES 32
#define BRICK_MAX_ELEMENTS (BRICK_MAX_BUTTONS + BRICK_MAX_SCROLLBOXES)

// General Style Settings
// ---------------------------------------------------------------
#define BRICK_STYLE_FONT_SIZE_DEFAULT 24
#define BRICK_STYLE_PADDING_SMALL 8
#define BRICK_STYLE_PADDING_MEDIUM 12
#define BRICK_STYLE_PADDING_LARGE 16

// Colors
// ---------------------------------------------------------------
#define BRICK_COLOR_BLANK       PLEX(Clay_Color){ 0, 0, 0, 0 }
#define BRICK_COLOR_WHITE       PLEX(Clay_Color){ 255, 255, 255, 255 }
#define BRICK_COLOR_BLACK       PLEX(Clay_Color){ 0, 0, 0, 255 }
#define BRICK_COLOR_BRICKRED    PLEX(Clay_Color){ 245, 245, 245, 255 }

#define BRICK_COLOR_GRAY_LIGHT  PLEX(Clay_Color){ 189, 195, 199, 255 }
#define BRICK_COLOR_GRAY        PLEX(Clay_Color){ 149, 165, 166, 255 }
#define BRICK_COLOR_GRAY_DARK   PLEX(Clay_Color){ 77, 77, 77, 255 }

#define BRICK_COLOR_BLACK_A80   PLEX(Clay_Color){ 0, 0, 0, 80 }

// Material UI Metro Colors - https://materialui.co/metrocolors
#define BRICK_COLOR_LIME        PLEX(Clay_Color){ 164, 196, 0, 255 }
#define BRICK_COLOR_GREEN       PLEX(Clay_Color){ 96, 169, 23, 255 }
#define BRICK_COLOR_EMERALD     PLEX(Clay_Color){ 0, 138, 0, 255 }
#define BRICK_COLOR_TEAL        PLEX(Clay_Color){ 0, 171, 169, 255 }
#define BRICK_COLOR_CYAN        PLEX(Clay_Color){ 27, 161, 226, 255 }
#define BRICK_COLOR_BLUE        PLEX(Clay_Color){ 41, 98, 255, 255 }
#define BRICK_COLOR_COBALT      PLEX(Clay_Color){ 0, 80, 239, 255 }
#define BRICK_COLOR_INDIGO      PLEX(Clay_Color){ 106, 0, 255, 255 }
#define BRICK_COLOR_VIOLET      PLEX(Clay_Color){ 170, 0, 255, 255 }
#define BRICK_COLOR_PINK        PLEX(Clay_Color){ 244, 114, 208, 255 }
#define BRICK_COLOR_MAGENTA     PLEX(Clay_Color){ 216, 0, 115, 255 }
#define BRICK_COLOR_CRIMSON     PLEX(Clay_Color){ 162, 0, 37, 255 }
#define BRICK_COLOR_RED         PLEX(Clay_Color){ 229, 20, 0, 255 }
#define BRICK_COLOR_ORANGE      PLEX(Clay_Color){ 250, 104, 0, 255 }
#define BRICK_COLOR_AMBER       PLEX(Clay_Color){ 240, 163, 10, 255 }
#define BRICK_COLOR_YELLOW      PLEX(Clay_Color){ 227, 200, 0, 255 }   
#define BRICK_COLOR_BROWN       PLEX(Clay_Color){ 130, 90, 44, 255 }
#define BRICK_COLOR_OLIVE       PLEX(Clay_Color){ 109, 135, 100, 255 }
#define BRICK_COLOR_STEEL       PLEX(Clay_Color){ 100, 118, 135, 255 }
#define BRICK_COLOR_MAUVE       PLEX(Clay_Color){ 118, 96, 138, 255 }
#define BRICK_COLOR_SIENNA      PLEX(Clay_Color){ 160, 82, 45, 255 }

// Theme 
// ---------------------------------------------------------------
#define BRICK_THEME_BACKGROUND  BRICK_COLOR_BLACK_A80
#define BRICK_THEME_FOREGROUND  BRICK_COLOR_GRAY_LIGHT
#define BRICK_THEME_PRIMARY     BRICK_COLOR_MAGENTA
#define BRICK_THEME_SECONDARY   BRICK_COLOR_MAUVE
#define BRICK_THEME_TERTIARY    BRICK_COLOR_OLIVE
#define BRICK_THEME_ACCENT      BRICK_COLOR_YELLOW

// Styles 
// ---------------------------------------------------------------

#define BRICK_STYLE_TEXT_DEFAULT    CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_FOREGROUND, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_LEFT })
#define BRICK_STYLE_TEXT_CENTERED   CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_FOREGROUND, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_CENTER })

// Theme Map 
// ---------------------------------------------------------------
#define BRICK_STYLE_BUTTON_LABEL            BRICK_STYLE_TEXT_DEFAULT
#define BRICK_COLOR_BUTTON_BORDER           BRICK_THEME_PRIMARY
#define BRICK_COLOR_BUTTON_BORDER_TOGGLE    BRICK_THEME_PRIMARY
#define BRICK_COLOR_BUTTON_BG               BRICK_THEME_BACKGROUND
#define BRICK_COLOR_BUTTON_BG_TOGGLE        BRICK_THEME_PRIMARY
#define BRICK_COLOR_BUTTON_BG_HOVER         BRICK_THEME_ACCENT

// ========================================================================================
//                                   PUBLIC HEADER
// ========================================================================================

#ifdef __cplusplus
extern "C" {
#endif

// Public Types
// --------------------------
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
    BRICK_ELEMENT_TYPE_NONE,
    BRICK_ELEMENT_TYPE_BUTTON,
    BRICK_ELEMENT_TYPE_TOGGLE_BUTTON,
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
    int32_t groupIndex;
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

// WARN: CHECK MAX COUNT WHEN ADDING EVENTS
#define BRICK_MAX_EVENT_TYPES 10
typedef CLAY_PACKED_ENUM {
    BRICK_EVENT_NONE,
    // Pointer enters hover
    // Duration: single frame
    BRICK_EVENT_HOVER,
    // Pointer is hovering
    // Duration: continuous frames
    BRICK_EVENT_HOVERING,
    // Pointer exits hover
    // Duration: single frame
    BRICK_EVENT_CLEAR,
    // Element is pressed and 
    // released quickly (click event)
    BRICK_EVENT_PRESS,
    // Element is being pressed
    // Duration: continuous frames
    BRICK_EVENT_PRESSING,
    // Element stopped being pressed
    // Duration: single frame
    BRICK_EVENT_RELEASE,
} Brick_EventType;

typedef struct Brick_Event {
    // internal index
    int32_t index;
    // type of element
    Brick_ElementType elementType;
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

// Public API
// --------------------------

// TODO: add the rest of the API prototypes
Brick_ElementId Brick_CreateButton(const char* label);
Brick_ElementId Brick_GroupButtons(const Brick_ElementId* buttonIds, int32_t groupSize);
Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index);

#endif /* BRICK_HEADER */

// ========================================================================================
//                                  IMPLEMENTATION
// ========================================================================================

#ifdef BRICK_IMPLEMENTATION
#undef BRICK_IMPLEMENTATION

// Element and Event Arrays
// --------------------------

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

// Global State
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

// event array passed back to user to handle events
static Brick_Event g_events[BRICK_MAX_ELEMENTS];
static int32_t g_events_last_length = 0;
// events snapshot array stores which events were triggered per frame
static bool g_is_events_snapshot_dirty = false;
static bool g_events_snapshot[BRICK_MAX_EVENT_TYPES] = CLAY__DEFAULT_STRUCT;

// Internal forward declarations
// ----------------------------------

void Brick_HandleError(Clay_ErrorData errorData);

// Getters and Setters
// ----------------------------------

Brick_Button* Brick_Button_Get(Brick_ElementId buttonId) {
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON && buttonId.type != BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) return &Brick_Button_DEFAULT;

    return Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);
}

// Global lifecycle
// ----------------------------------
void Brick_Initialize(float width, float height, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void *fontData), void *fontData) {
    // initializes Clay first, then Brick
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

void Brick_Destroy(void) {
    if(g_clay_arena.memory) free(g_clay_arena.memory);
}

// CreateElement<Element>
// initializes the element state and returns the element ID for layout
// ---------------------------------------------------------------------------

Brick_ElementId Brick_CreateElementId(int32_t index, Brick_ElementType type) {
    Brick_ElementId id = { index, type };
    return id;
}

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
        .groupIndex = 0,
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

Brick_ElementId Brick_CreateToggleButton(const char* label) {
    Brick_ElementId buttonId = Brick_CreateButton(label);

    Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonId.index);

    Brick_ElementId toggleButtonId = PLEX(Brick_ElementId){ buttonId.index, BRICK_ELEMENT_TYPE_TOGGLE_BUTTON };
    button->id = toggleButtonId;
    
    return toggleButtonId;
}

Brick_ElementId Brick_CreateButtonGroup(const Brick_ElementId* buttonIds, int32_t groupSize) {

    // get the next index to store in button
    int32_t index = g_elements.buttonGroups.length;
    // default group init
    Brick_ElementGroup group = CLAY__DEFAULT_STRUCT;

    // iterate over the button ids
    for (int32_t i = 0; i < groupSize; i++) {
        if (buttonIds[i].type != BRICK_ELEMENT_TYPE_BUTTON && buttonIds[i].type != BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) {
            // TODO: exit or handle error 
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
        }

        Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, buttonIds[i].index);

        if (button->id.index == 0) {
            // TODO: exit or handle error 
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
        }

        // cross reference the group
        button->groupIndex = index;
        if (i == 0 && button->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) {
            button->toggled = true;
        }
        // store the button id in the group
        group.ids[i] = button->id.index;
        group.length++;
    }

    // if all buttons are valid, store the group
    g_elements.buttonGroups.data[index] = group;
    g_elements.buttonGroups.length++;

    return Brick_CreateElementId(index, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
}

// Update and queries
// -------------------------------------------------------------------------

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
Brick_EventArray Brick_UpdateEvents(Brick_PointerData pointerData) {

    Clay_SetPointerState(Clay_Vector2({ .x = pointerData.x, .y = pointerData.y }), pointerData.pressed);
    
    Brick_EventArray events = {
        .length = 0,
        .data = g_events
    };

    // clear the events snapshot
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
                .index = i,
                .elementType = button->id.type,
                .eventType = BRICK_EVENT_PRESS
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_PRESS] = true;
        } 
        else if (button->pressed) { 
            g_events[events.length] = {
                .index = i,
                .elementType = button->id.type,
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
                .index = i,
                .elementType = button->id.type,
                .eventType = BRICK_EVENT_RELEASE
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_RELEASE] = true;
        }
        else if(button->hovered) {
            Brick_EventType eventType = Brick_PointerJustHovered() ? BRICK_EVENT_HOVER : BRICK_EVENT_HOVERING;
            g_events[events.length] = {
                .index = i,
                .elementType = button->id.type,
                .eventType = eventType
            };
            events.length++;

            g_events_snapshot[eventType] = true;
        }
        else if(button->cleared) {
            g_events[events.length] = {
                .index = i,
                .elementType = button->id.type,
                .eventType = BRICK_EVENT_CLEAR
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_CLEAR] = true;
        }
    }

    // update the length cache for querying events
    g_events_last_length = events.length;

    // flag the events snapshot for clear
    if (events.length) g_is_events_snapshot_dirty = true;

    return events;
}

Brick_EventArray Brick_PollEvents(void) {
    Brick_EventArray events = {
        .length = g_events_last_length,
        .data = g_events
    };

    return events;
}

bool Brick_IsEventTriggeredById(Brick_EventType eventType, Brick_ElementId elementId) {
    // TODO: add some error handling
    if (eventType > BRICK_MAX_EVENT_TYPES) return false;
    
    for (int32_t i = 0; i < g_events_last_length; i++) {
        if (g_events[i].eventType == eventType && g_events[i].elementType == elementId.type && g_events[i].index == elementId.index) {
            return true;
        }
    }

    return false;
}

bool Brick_IsEventTriggered(Brick_EventType eventType) {
    // TODO: add some error handling
    if (eventType > BRICK_MAX_EVENT_TYPES) return false;
    
    return g_events_snapshot[eventType];
}

bool Brick_IsButtonToggled(const Brick_ElementId buttonId) {
    Brick_Button* button = Brick_Button_Get(buttonId);

    return button->toggled;
}

void Brick_ToggleButton(Brick_ElementId buttonId) {
    Brick_Button* button = Brick_Button_Get(buttonId);

    button->toggled = !button->toggled;
}

void Brick_ToggleButton_Set(Brick_ElementId buttonId, bool isToggled) {
    Brick_Button* button = Brick_Button_Get(buttonId);

    button->toggled = isToggled;
}

// ========================================================================================
//                                      LAYOUT
// ========================================================================================

// simple wrapper around Clay_BeginLayout
void Brick_BeginLayout(void) {
    Clay_BeginLayout();
}

// simple wrapper around Clay_EndLayout which returns render commands
Clay_RenderCommandArray Brick_EndLayout(float deltaTime) {
    return Clay_EndLayout(deltaTime);
}

// Inline<Element>
// called inside containers with literal values
// ---------------------------------------------------------------

void Brick_InlineText(const char* text) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(text), 
        .chars = text 
    };

    CLAY_TEXT(clayString, BRICK_STYLE_TEXT_DEFAULT);
}

// Layout<Element>
// called inside containers with Begin and End
// ---------------------------------------------------------------

// TODO: add createText, store text state and use ID here
void Brick_LayoutText(const char* text) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(text), 
        .chars = text 
    };

    CLAY_TEXT(clayString, BRICK_STYLE_TEXT_DEFAULT);
}

// Button handlers
// ----------------------------------
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
                // if button is part of a group clear the toggled buttons
                if (button->groupIndex > 0) {
                    Brick_ElementGroup* buttonGroup = Brick_ButtonGroupArray_Get(&g_elements.buttonGroups, button->groupIndex);
                    for(int32_t j = 0; j < buttonGroup->length; j++) {
                        int32_t buttonIdx = buttonGroup->ids[j];
                        Brick_Button* groupBtn = Brick_ButtonArray_Get(&g_elements.buttons, buttonIdx);
                        if (groupBtn->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON){
                            groupBtn->toggled = false;
                        }
                    }
                }
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
                // NOTE: This is almost the same as hover, Clay triggers this if pointer 
                // is on the button not pressing, and after pressing
                break;
            default: break;
            }
            break;
        }
    }
}

// internal button layout function using internal index
void Brick__LayoutButtonIndex(int32_t index) {

    const Brick_Button* button = Brick_ButtonArray_Get(&g_elements.buttons, index);
    Clay_Color bgColor = button->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON && button->toggled ? BRICK_COLOR_BUTTON_BG_TOGGLE : BRICK_COLOR_BUTTON_BG;
    Clay_Color borderColor = button->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON && button->toggled ? BRICK_COLOR_BUTTON_BORDER_TOGGLE : BRICK_COLOR_BUTTON_BORDER;

    CLAY(button->clayId, {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0)
            },
            .padding = {
                BRICK_STYLE_PADDING_SMALL,
                BRICK_STYLE_PADDING_SMALL,
                BRICK_STYLE_PADDING_MEDIUM,
                BRICK_STYLE_PADDING_MEDIUM
            },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
        }, 
        // Clay_Hovered only works inside the paramaters or declaration body
        .backgroundColor = Clay_Hovered() ? BRICK_COLOR_BUTTON_BG_HOVER : bgColor,
        .border = { 
            .color = borderColor, 
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
        CLAY_TEXT(button->label, BRICK_STYLE_BUTTON_LABEL);
    }
}

void Brick_LayoutButton(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON && buttonId.type != BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) return;

    Brick__LayoutButtonIndex(buttonId.index);
}

// TODO: add Group_Get to consolidate error checking
void Brick_LayoutButtonGroup(Brick_ElementId groupId) {
    // TODO: add error handling
    if (groupId.type != BRICK_ELEMENT_TYPE_BUTTON_GROUP) return;

    const Brick_ElementGroup* buttonGroup = Brick_ButtonGroupArray_Get(&g_elements.buttonGroups, groupId.index);

    for (int32_t i = 0; i < buttonGroup->length; i++) {
        Brick__LayoutButtonIndex(buttonGroup->ids[i]);
    }
}

// Layout Containers
// BeginLayout<Element> and EndLayout<Element>
// ---------------------------------------------------------------------------------------------

void Brick_BeginPanel(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .padding = CLAY_PADDING_ALL(BRICK_STYLE_PADDING_SMALL), 
        },
        .backgroundColor = BRICK_THEME_BACKGROUND,
    });
}

void Brick_EndPanel(void) {
    Clay__CloseElement();
}

void Brick_BeginFloatingPanel(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(0.5f),
                .height = CLAY_SIZING_PERCENT(0.5f),
            },
            .padding = CLAY_PADDING_ALL(BRICK_STYLE_PADDING_SMALL), 
            .childGap = BRICK_STYLE_PADDING_SMALL, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = BRICK_THEME_BACKGROUND,
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

void Brick_EndFloatingPanel(void) {
    Clay__CloseElement();
}

void Brick_BeginHorizontalStack(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0),
            },
            .childGap = BRICK_STYLE_PADDING_SMALL, 
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, 
            .layoutDirection = CLAY_LEFT_TO_RIGHT 
        },
    });
}

void Brick_EndHorizontalStack(void) {
    Clay__CloseElement();
}

void Brick_BeginVerticalStack(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .height = CLAY_SIZING_GROW(0),
            },
            .childGap = BRICK_STYLE_PADDING_SMALL, 
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER }, 
            .layoutDirection = CLAY_LEFT_TO_RIGHT 
        },
    });
}

void Brick_EndVerticalStack(void) {
    Clay__CloseElement();
}

// ========================================================================================
//                                     ERROR HANDLING
// ========================================================================================

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
