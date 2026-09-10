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
// ####################################^########################################
//                                  LIBRARY
// #############################################################################

// ------------------------------------.----------------------------------------
//                                  CONTENT
// =============================================================================

//                                  Section
// ------------------------------------.----------------------------------------

// Sub-Section
// _____________________________________________________________________________

#ifdef BRICK_IMPLEMENTATION
#define CLAY_IMPLEMENTATION
#endif
#include "clay.h"

// ####################################^########################################
//                               PUBLIC HEADER
// #############################################################################

#ifndef BRICK_HEADER
#define BRICK_HEADER

#if defined(__cplusplus)
    #define PLEX(type) type
#else
    #define PLEX(type) (type)
#endif

// ------------------------------------.----------------------------------------
//                                 SETTINGS
// =============================================================================

// Max Elements and Containers 
// _____________________________________________________________________________
// Determines the element and container type arrays max length
// Arrays are initialized as static global arrays
#define BRICK_MAX_TEXTS 128
// MAX_BUTTONS should not exceed GROUP_SIZE * GROUPS
#define BRICK_MAX_BUTTONS 128
#define BRICK_MAX_BUTTON_GROUP_SIZE 16
#define BRICK_MAX_BUTTON_GROUPS 8
// IMAGE_BUTTONS cannot be grouped
#define BRICK_MAX_IMAGE_BUTTONS 64
// total max number of elements
#define BRICK_MAX_ELEMENTS (BRICK_MAX_TEXTS + BRICK_MAX_BUTTONS + BRICK_MAX_IMAGE_BUTTONS)

#define BRICK_MAX_SCROLLBOXES 32
// total max number of containers
#define BRICK_MAX_CONTAINERS BRICK_MAX_SCROLLBOXES

// General Global Styles 
// _____________________________________________________________________________
#define BRICK_STYLE_FONT_SIZE_DEFAULT 24
#define BRICK_STYLE_PADDING_SMALL 8
#define BRICK_STYLE_PADDING_MEDIUM 12
#define BRICK_STYLE_PADDING_LARGE 16

// Colors 
// _____________________________________________________________________________
#define BRICK_COLOR_BLANK       PLEX(Clay_Color){ 0, 0, 0, 0 }
#define BRICK_COLOR_WHITE       PLEX(Clay_Color){ 255, 255, 255, 255 }
#define BRICK_COLOR_BLACK       PLEX(Clay_Color){ 0, 0, 0, 255 }
#define BRICK_COLOR_BRICK       PLEX(Clay_Color){ 144, 30, 30, 255 }

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
// _____________________________________________________________________________
#define BRICK_THEME_BACKGROUND  BRICK_COLOR_BLACK_A80
#define BRICK_THEME_FOREGROUND  BRICK_COLOR_BRICK
#define BRICK_THEME_PRIMARY     BRICK_COLOR_GRAY_LIGHT
#define BRICK_THEME_SECONDARY   BRICK_COLOR_ORANGE
#define BRICK_THEME_TERTIARY    BRICK_COLOR_OLIVE
#define BRICK_THEME_ACCENT      BRICK_COLOR_SIENNA

// Styles
// _____________________________________________________________________________
#define BRICK_STYLE_TEXT_DEFAULT    CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_PRIMARY, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_LEFT })
#define BRICK_STYLE_TEXT_CENTERED   CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_PRIMARY, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_CENTER })

// Theme-Style Mapping 
// _____________________________________________________________________________
#define BRICK_STYLE_BUTTON_LABEL            BRICK_STYLE_TEXT_DEFAULT
#define BRICK_COLOR_BUTTON_BORDER           BRICK_THEME_TERTIARY
#define BRICK_COLOR_BUTTON_BORDER_TOGGLE    BRICK_THEME_TERTIARY
#define BRICK_COLOR_BUTTON_BG               BRICK_THEME_FOREGROUND
#define BRICK_COLOR_BUTTON_BG_TOGGLE        BRICK_THEME_SECONDARY
#define BRICK_COLOR_BUTTON_BG_HOVER         BRICK_THEME_ACCENT


// ------------------------------------.----------------------------------------
//                               PUBLIC TYPES
// =============================================================================

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
    float scrollX;
    float scrollY;
    bool pressed;
    bool released;
} Brick_PointerData;

typedef CLAY_PACKED_ENUM {
    BRICK_CONTAINER_TYPE_NONE,
    BRICK_CONTAINER_TYPE_SCROLLBOX,
} Brick_ContainerType;

typedef struct Brick_ContainerId {
    int32_t index;
    Brick_ContainerType type;
} Brick_ContainerId;

typedef struct Brick_ScrollBox {
    Clay_ElementId clayId;
    Clay_ElementId clayParentId;
    Brick_ContainerId id;
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    float scrollY;
    bool isPrimaryDown;
} Brick_ScrollBox;

typedef CLAY_PACKED_ENUM {
    BRICK_ELEMENT_TYPE_NONE,
    BRICK_ELEMENT_TYPE_TEXT,
    BRICK_ELEMENT_TYPE_BUTTON,
    BRICK_ELEMENT_TYPE_TOGGLE_BUTTON,
    BRICK_ELEMENT_TYPE_IMAGE_BUTTON,
    BRICK_ELEMENT_TYPE_BUTTON_GROUP,
} Brick_ElementType;

typedef struct Brick_ElementId {
    int32_t index;
    Brick_ElementType type;
} Brick_ElementId;

typedef struct Brick_Text {
    // Clay_ElementId clayId;
    Clay_String clayString;
    Brick_ElementId id;
    int32_t fontSize;
} Brick_Text;

typedef struct Brick_ButtonState {
    bool hovered;
    bool cleared;
    bool clicked;
    bool pressed;
    bool released;
    bool toggled;
} Brick_ButtonState;

typedef struct {
    Clay_ElementId clayId;
    Clay_String label;
    Brick_ElementId id;
    void* imageData;
    Brick_ButtonState state;
    int32_t groupIndex;
    float width;
    float height;
    int32_t fontSize;
} Brick_Button;

typedef struct {
    Brick_ElementId id;
    void* imageData;
    Brick_ButtonState state;
    int32_t groupIndex;
    float width;
    float height;
} Brick_ImageButton;

// TODO: rename ids to indices?
typedef struct {
    int32_t length;
    int32_t ids[BRICK_MAX_BUTTON_GROUP_SIZE];
} Brick_ElementGroup;

// WARN: CHECK MAX COUNT WHEN ADDING EVENTS
#define BRICK_MAX_EVENT_TYPES 10
typedef CLAY_PACKED_ENUM {
    BRICK_EVENT_TYPE_NONE,
    // Pointer enters hover
    // Duration: single frame
    BRICK_EVENT_TYPE_HOVER,
    // Pointer is hovering
    // Duration: continuous frames
    BRICK_EVENT_TYPE_HOVERING,
    // Pointer exits hover
    // Duration: single frame
    BRICK_EVENT_TYPE_CLEAR,
    // Element is pressed and 
    // released quickly (click event)
    BRICK_EVENT_TYPE_PRESS,
    // Element is being pressed
    // Duration: continuous frames
    BRICK_EVENT_TYPE_PRESSING,
    // Element stopped being pressed
    // Duration: single frame
    BRICK_EVENT_TYPE_RELEASE,
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

// ------------------------------------.----------------------------------------
//                                PUBLIC API
// =============================================================================
// Sections:
// Lifecycle  - manages the lifetime and/or state of the library
// Events     - querying and updating element and container events
// Elements   - creating and rendering the layout of elements
// Containers - creating and rendering the layout of containers
// 
// Sorting per section:
// - Getters  - retrieving without mutating global state
// - Setters  - modifies global state
// - Create   - adds new instances on the global state
// - Update   - called every frame updating global state
// - Layout   - calls Clay macros every frame to render layout

//                                 Lifecycle
// ------------------------------------.----------------------------------------
// Initializes Clay and other global state. The MeasureText function is passed 
// through directly to Clay_SetMeasureTextFunction along with the font data.
void Brick_Initialize(float width, float height, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice text, Clay_TextElementConfig *config, void *fontData), void *fontData);
// Calls Clay_SetLayoutDimensions to recalculate positioning of elements on the 
// screen. TODO: Resize will also recalculate styles based on native resolution. 
void Brick_Resize(float width, float height);
// Cleans up Clay arena. Global state is cleaned up by OS on exit. 
void Brick_Destroy(void);
// Simple wrapper around Clay_BeginLayout and ClayEndLayout
void Brick_BeginLayout(void);
Clay_RenderCommandArray Brick_EndLayout(float deltaTime);

//                                   Events
// ------------------------------------.----------------------------------------
// Checks whether the given event type was triggered by the given element
// inside of the render-update loop, i.e. it will check during each frame.
bool Brick_IsEventTriggeredById(Brick_EventType eventType, Brick_ElementId elementId);
// Checks whether the given event was triggered at all across any element
// globally. Events are aggregated each frame for all elements.
bool Brick_IsEventTriggered(Brick_EventType eventType);
// Clay-like interface for retrieving an array of events and iterating over them.
Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index);
// An alternative way of getting the events during each frame. This is meant
// to give a way to access events across different files and translation units
// without updating events twice during a single frame.
Brick_EventArray Brick_PollEvents(void);
// Updates the global state for all elements and containers, and aggregates
// events for querying. This is meant to be called every frame before Layout
Brick_EventArray Brick_UpdateEvents(Brick_PointerData pointerData, float deltaTime);

//                                  Elements
// ------------------------------------.----------------------------------------
// Inline<Element> takes literal values and does not store any state or IDs
// Create<Element> takes configuration arguments and returns an ID
// Layout<Element> takes IDs and configures the element and updates state

// Text
void Brick_InlineText(const char* text);
Brick_ElementId Brick_CreateText(const char* text);
void Brick_LayoutText(Brick_ElementId textId);

// Button
bool Brick_IsButtonToggled(const Brick_ElementId buttonId);
void Brick_ToggleButton(Brick_ElementId buttonId);
void Brick_ToggleButton_Set(Brick_ElementId buttonId, bool isToggled);
Brick_ElementId Brick_CreateButton(const char* label);
// TODO: implement
// Brick_ElementId Brick_CreateButtonEx(const char* label, int32_t width, int32_t height, int32_t fontSize, void* imageData);
Brick_ElementId Brick_CreateToggleButton(const char* label);
void Brick_LayoutButton(Brick_ElementId buttonId);

// Image Button
Brick_ElementId Brick_CreateImageButton(float width, float height, void* imageData);
void Brick_LayoutImageButton(Brick_ElementId buttonId);

// Button Group
Brick_ElementId Brick_CreateButtonGroup(const Brick_ElementId* buttonIds, int32_t groupSize);
void Brick_LayoutButtonGroup(Brick_ElementId groupId);

//                                Containers
// ------------------------------------.----------------------------------------
// Begin<Container> requires closing with End<Container>

// Stateful Containers
// _____________________________________________________________________________
// adds a global state instance, Create<Container> is required 

// Scroll Box
Brick_ContainerId Brick_CreateScrollBox(void);
void Brick_BeginScrollBox(Brick_ContainerId scrollBoxId);
void Brick_EndScrollBox();

// Stateless Containers
// _____________________________________________________________________________
// Does not require creation or ID management

// Panel
void Brick_BeginPanel(void);
void Brick_EndPanel(void);

// Floating Panel
void Brick_BeginFloatingPanel(void);
void Brick_EndFloatingPanel(void);

// Horizontal Stack
void Brick_BeginHorizontalStack(void);
void Brick_EndHorizontalStack(void);

// Vertical Stack
void Brick_BeginVerticalStack(void);
void Brick_EndVerticalStack(void);

// Offset
void Brick_BeginOffset(float x, float y);
void Brick_EndOffset(void);

#endif /* BRICK_HEADER */

// ####################################^########################################
//                              IMPLEMENTATION
// #############################################################################

#ifdef BRICK_IMPLEMENTATION
#undef BRICK_IMPLEMENTATION

// ------------------------------------.----------------------------------------
//                               PRIVATE HEADER
// =============================================================================

//                                Array Types
// ------------------------------------.----------------------------------------

// Elements
// _____________________________________________________________________________
typedef struct Brick_TextArray {
    int32_t length;
    Brick_Text* data;
} Brick_TextArray;

typedef struct Brick_ButtonArray {
    int32_t length;
    Brick_Button* data;
} Brick_ButtonArray;

typedef struct Brick_ImageButtonArray {
    int32_t length;
    Brick_ImageButton* data;
} Brick_ImageButtonArray;

typedef struct Brick_ButtonGroupArray {
    int32_t length;
    Brick_ElementGroup* data;
} Brick_ButtonGroupArray;

typedef struct Brick_Elements {
    int32_t total_count;
    Brick_TextArray texts;
    Brick_ButtonArray buttons;
    Brick_ImageButtonArray imageButtons;
    Brick_ButtonGroupArray buttonGroups;
} Brick_Elements;

// Containers
// _____________________________________________________________________________
typedef struct Brick_ContainerStackArray {
    int32_t length;
    int32_t* data;
} Brick_ContainerStackArray;

typedef struct Brick_ScrollBoxArray {
    int32_t length;
    Brick_ScrollBox* data;
} Brick_ScrollBoxArray;

typedef struct Brick_Containers {
    int32_t total_count;
    Brick_ContainerStackArray stack;
    Brick_ScrollBoxArray scrollBoxes;
} Brick_Containers;

//                               Global State
// ------------------------------------.----------------------------------------
// TODO: should arrays be initialized with CLAY__DEFAULT_STRUCT?

// Clay context
static Clay_Arena g_clay_arena = CLAY__DEFAULT_STRUCT;
// NOTE: window state also holds some pointer state
static Brick_Window g_window = CLAY__DEFAULT_STRUCT;

// Events
static int32_t g_events_last_length = 0;
// event array passed back to user to handle events
static Brick_Event g_events[BRICK_MAX_ELEMENTS];
// events snapshot array stores which events were triggered per frame
static bool g_is_events_snapshot_stale = false;
static bool g_events_snapshot[BRICK_MAX_EVENT_TYPES] = CLAY__DEFAULT_STRUCT;

// Elements
static Brick_Text g_texts[BRICK_MAX_TEXTS];
static Brick_Button g_buttons[BRICK_MAX_BUTTONS];
static Brick_ImageButton g_image_buttons[BRICK_MAX_IMAGE_BUTTONS];
static Brick_ElementGroup g_button_groups[BRICK_MAX_BUTTON_GROUPS];
static Brick_Elements g_elements = {
    .total_count = 0,
    .texts = {
        .length = 0,
        .data = g_texts
    },
    .buttons = {
        .length = 0,
        .data = g_buttons
    },
    .imageButtons = {
        .length = 0,
        .data = g_image_buttons
    },
    .buttonGroups = {
        .length = 0,
        .data = g_button_groups
    },
};

// Containers
static int32_t g_container_stack[BRICK_MAX_CONTAINERS];
static Brick_ScrollBox g_scroll_boxes[BRICK_MAX_SCROLLBOXES];
static Brick_Containers g_containers = {
    .total_count = 0,
    .stack = {
        .length = 0,
        .data = g_container_stack
    },
    .scrollBoxes = {
        .length = 0,
        .data = g_scroll_boxes
    },
};

// default global placeholders
Brick_Event Brick_Event_DEFAULT                 = CLAY__DEFAULT_STRUCT;
Brick_Text Brick_Text_DEFAULT                   = CLAY__DEFAULT_STRUCT;
Brick_ButtonState Brick_ButtonState_DEFAULT     = CLAY__DEFAULT_STRUCT;
Brick_Button Brick_Button_DEFAULT               = CLAY__DEFAULT_STRUCT;
Brick_ImageButton Brick_ImageButton_DEFAULT     = CLAY__DEFAULT_STRUCT;
Brick_ElementGroup Brick_ElementGroup_DEFAULT   = CLAY__DEFAULT_STRUCT;
Brick_ScrollBox Brick_ScrollBox_DEFAULT         = CLAY__DEFAULT_STRUCT;

//                               Array Getters
// ------------------------------------.----------------------------------------
Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_Event_DEFAULT;
}

Brick_Text* Brick_Text_IndexGet(int32_t index) {                                                    
    return index < g_elements.texts.length && index >= 0 ? &g_elements.texts.data[index] : &Brick_Text_DEFAULT;
}

Brick_Button* Brick_Button_IndexGet(int32_t index) {                                                    
    return index < g_elements.buttons.length && index >= 0 ? &g_elements.buttons.data[index] : &Brick_Button_DEFAULT;
}    

Brick_ImageButton* Brick_ImageButton_IndexGet(int32_t index) {                                                    
    return index < g_elements.imageButtons.length && index >= 0 ? &g_elements.imageButtons.data[index] : &Brick_ImageButton_DEFAULT;
}

Brick_ElementGroup* Brick_ButtonGroup_IndexGet(int32_t index) {                                                    
    return index < g_elements.buttonGroups.length && index >= 0 ? &g_elements.buttonGroups.data[index] : &Brick_ElementGroup_DEFAULT;
}

// NOTE: not used:
Brick_Button* Brick_Button_Get(Brick_ElementId buttonId) {
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON && buttonId.type != BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) return &Brick_Button_DEFAULT;

    return Brick_Button_IndexGet(buttonId.index);
}

Brick_ButtonState* Brick_ButtonState_Get(Brick_ElementId buttonId) {
    Brick_ButtonState* buttonState = &Brick_ButtonState_DEFAULT;
    
    if (buttonId.type == BRICK_ELEMENT_TYPE_BUTTON || buttonId.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) {
        Brick_Button* button = Brick_Button_IndexGet(buttonId.index);
        return &button->state;
    } else if (buttonId.type == BRICK_ELEMENT_TYPE_IMAGE_BUTTON) {
        Brick_ImageButton* button = Brick_ImageButton_IndexGet(buttonId.index);
        return &button->state;
    }

    return buttonState;
}

Brick_ScrollBox* Brick_ScrollBox_IndexGet(int32_t index) {
    return index < g_containers.scrollBoxes.length && index >= 0 ? &g_containers.scrollBoxes.data[index] : &Brick_ScrollBox_DEFAULT;
}

//                               Array Setters
// ------------------------------------.----------------------------------------
int32_t Brick_ContainerStack_Pop(void) {
    if (g_containers.stack.length <= 0) return -1;

    g_containers.stack.length--;
    return g_containers.stack.data[g_containers.stack.length];
}

void Brick_ContainerStack_Push(int32_t index) {
    g_containers.stack.data[g_containers.stack.length] = index;
    g_containers.stack.length++;
}

//                             Private Prototypes
// ------------------------------------.----------------------------------------
void Brick_HandleError(Clay_ErrorData errorData);

// ------------------------------------.----------------------------------------
//                               API FUNCTIONS
// =============================================================================

//                                 Lifecycle
// ------------------------------------.----------------------------------------
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
    Brick_CreateImageButton(0, 0, nullptr);
    // bypassing Brick_GroupButtons that checks 0 as invalid
    g_elements.buttonGroups.data[0] = Brick_ElementGroup_DEFAULT;
    g_elements.buttonGroups.length++;
}

void Brick_Resize(float width, float height) {
    g_window.width = width;
    g_window.height = height;
    Clay_SetLayoutDimensions(Clay_Dimensions({ width, height }));
}

void Brick_Destroy(void) {
    if(g_clay_arena.memory) free(g_clay_arena.memory);
}

// simple wrapper around Clay_BeginLayout
void Brick_BeginLayout(void) {
    Clay_BeginLayout();
}

// simple wrapper around Clay_EndLayout which returns render commands
Clay_RenderCommandArray Brick_EndLayout(float deltaTime) {
    return Clay_EndLayout(deltaTime);
}

//                                   Events
// ------------------------------------.----------------------------------------
// Global pointer hover check on any button. This is meant to be used in UpdateEvents.
// WARN: using this function by itself can be a race condition with the button HoverHandler
bool Brick_PointerJustHovered() {
    return g_window.hoveredId != 0 && g_window.lastHoveredId != g_window.hoveredId;
}
// Global pointer hover clear check on any button. This is meant to be used in UpdateEvents.
// WARN: using this function by itself can be a race condition with the button HoverHandler
bool Brick_PointerJustCleared() {
    return g_window.hoveredId == 0 && g_window.lastHoveredId != 0;
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
    if (eventType >= BRICK_MAX_EVENT_TYPES) return false;
    
    return g_events_snapshot[eventType];
}

Brick_EventArray Brick_PollEvents(void) {
    Brick_EventArray events = {
        .length = g_events_last_length,
        .data = g_events
    };

    return events;
}

// Brick only function that will handle any potential updates of elements per frame
Brick_EventArray Brick_UpdateEvents(Brick_PointerData pointerData, float deltaTime) {

    Clay_SetPointerState(Clay_Vector2({ .x = pointerData.x, .y = pointerData.y }), pointerData.pressed);
    
    Brick_EventArray events = {
        .length = 0,
        .data = g_events
    };

    // clear the events snapshot
    if (g_is_events_snapshot_stale) {
        for (int32_t i = 0; i < BRICK_MAX_EVENT_TYPES; i++) {
            g_events_snapshot[i] = false;
        }
    }

    // Button Events ------------------------------------
    // skip unit button at index 0
    for (int32_t i = 1; i < g_elements.buttons.length; i++) {
        Brick_Button* button = Brick_Button_IndexGet(i);
        Brick_ButtonState* state = &button->state;
        Brick_ElementType buttonType = button->id.type;

        if(state->clicked && !state->pressed) {
            // prevents event from firing after button is
            // not rendered, i.e. clicking to change panels
            state->clicked = false;
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_PRESS
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_PRESS] = true;
            // simulate the hover clear on click in case the
            // click stops rendering the current button 
            // (prevents HOVERING event sticking, i.e. always showing hand cursor)
            g_events_snapshot[BRICK_EVENT_TYPE_CLEAR] = true;
        } 
        else if (state->pressed) { 
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_PRESSING
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_PRESSING] = true;
        }
        else if(state->released) {
            // prevents from firing after button is
            // blocked or not rendered, i.e. showing a popup window
            state->released = false;
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_RELEASE
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_RELEASE] = true;
            // reverses the HOVER clear simulation on click (PRESS)
            // by setting HOVER back to true and allowing HOVER to trigger
            // again if the button is still being rendered in the layout
            g_events_snapshot[BRICK_EVENT_TYPE_HOVER] = true;
        }
        else if(state->hovered) {
            Brick_EventType eventType = Brick_PointerJustHovered() ? BRICK_EVENT_TYPE_HOVER : BRICK_EVENT_TYPE_HOVERING;
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = eventType
            };
            events.length++;

            g_events_snapshot[eventType] = true;
        }
        else if(state->cleared) {
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_CLEAR
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_CLEAR] = true;
        }
    }

    for (int32_t i = 1; i < g_elements.imageButtons.length; i++) {
        Brick_ImageButton* button = Brick_ImageButton_IndexGet(i);
        Brick_ButtonState* state = &button->state;
        Brick_ElementType buttonType = button->id.type;

        if(state->clicked && !state->pressed) {
            // prevents event from firing after button is
            // not rendered, i.e. clicking to change panels
            state->clicked = false;
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_PRESS
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_PRESS] = true;
        } 
        else if (state->pressed) { 
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_PRESSING
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_PRESSING] = true;
        }
        else if(state->released) {
            // prevents from firing after button is
            // blocked or not rendered, i.e. showing a popup window
            state->released = false;
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_RELEASE
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_RELEASE] = true;
        }
        else if(state->hovered) {
            Brick_EventType eventType = Brick_PointerJustHovered() ? BRICK_EVENT_TYPE_HOVER : BRICK_EVENT_TYPE_HOVERING;
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = eventType
            };
            events.length++;

            g_events_snapshot[eventType] = true;
        }
        else if(state->cleared) {
            g_events[events.length] = {
                .index = i,
                .elementType = buttonType,
                .eventType = BRICK_EVENT_TYPE_CLEAR
            };
            events.length++;

            g_events_snapshot[BRICK_EVENT_TYPE_CLEAR] = true;
        }
    }

    // update Clay scroll containers once for all scrollboxes
    if (g_containers.scrollBoxes.length > 0) {
        Clay_UpdateScrollContainers(true, Clay_Vector2({ pointerData.scrollX*2.0f, pointerData.scrollY*2.0f }), deltaTime);
    }

    for (int32_t i = 0; i < g_containers.scrollBoxes.length; i++) {
        Brick_ScrollBox* scrollBox = Brick_ScrollBox_IndexGet(i);
        Clay_ScrollContainerData container = Clay_GetScrollContainerData(scrollBox->clayParentId);
        
        // WARNING: crashes without this check!
        if(container.scrollPosition) {
            // update the vertical scroll movement for mouse wheel, and mouse grab (content drag)
            scrollBox->scrollY = container.scrollPosition->y - scrollBox->positionOrigin.y;
        }

        if (pointerData.released) {
            scrollBox->isPrimaryDown = false;
            continue;
        }

        if (pointerData.pressed && !scrollBox->isPrimaryDown && Clay_PointerOver(scrollBox->clayId)) {

            scrollBox->clickOrigin = PLEX(Clay_Vector2){ pointerData.x, pointerData.y };
            scrollBox->positionOrigin = *container.scrollPosition;
            scrollBox->isPrimaryDown = true;

        } else if (scrollBox->isPrimaryDown) {
            // TODO: fix pulling the content too far up or too far down causing the scrollbar to go beyond the content
            // and if there is an image background, it offsets it too much and causes glitching
            if (container.contentDimensions.height > 0) {
                Clay_Vector2 ratio = Clay_Vector2({
                    container.contentDimensions.width / container.scrollContainerDimensions.width,
                    container.contentDimensions.height / container.scrollContainerDimensions.height,
                });

                if (container.config.vertical) {
                    container.scrollPosition->y = scrollBox->positionOrigin.y + (scrollBox->clickOrigin.y - pointerData.y) * ratio.y;
                }
                
                if (container.config.horizontal) {
                    container.scrollPosition->x = scrollBox->positionOrigin.x + (scrollBox->clickOrigin.x - pointerData.x) * ratio.x;
                }
            }
        }
    }

    // update the length cache for querying events
    g_events_last_length = events.length;

    // flag the events snapshot for clear
    if (events.length) g_is_events_snapshot_stale = true;

    return events;
}

//                              Element Functions
// ------------------------------------.----------------------------------------

Clay_TransitionData FadeSlide(Clay_TransitionData initialState, Clay_TransitionProperty properties) {
    Clay_TransitionData targetState = initialState;
    // small slide-in effect 
    if (properties & CLAY_TRANSITION_PROPERTY_POSITION) {
        targetState.boundingBox.y = targetState.boundingBox.y - 10.0f;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR) {
        targetState.backgroundColor.a = 0.0f;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_BORDER_COLOR) {
        targetState.borderColor.b = 0.0f;
    }
    return targetState;
}

//                                 Elements
// ------------------------------------.----------------------------------------
// CreateElement<Element> - initializes the element state and returns the element ID for layout
// Inline<Element> - called inside containers with literal values
// Layout<Element> - called inside containers with Begin and End

Brick_ElementId Brick_CreateElementId(int32_t index, Brick_ElementType type) {
    Brick_ElementId id = { index, type };
    return id;
}

// Text
// _____________________________________________________________________________

void Brick_InlineText(const char* text) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(text), 
        .chars = text 
    };

    CLAY_TEXT(clayString, BRICK_STYLE_TEXT_DEFAULT);
}

Brick_ElementId Brick_CreateText(const char* text) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(text), 
        .chars = text 
    };

    int32_t index = g_elements.texts.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_TEXTS) return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_NONE);

    Brick_ElementId textId = {
        .index = index,
        .type = BRICK_ELEMENT_TYPE_TEXT,
    };

    Brick_Text new_text = {
        // TODO: review if text needs container and an clay element Id
        // .clayId = CLAY_SID(clayString),
        .clayString = clayString,
        .id = textId,
        .fontSize = 0,
    };

    g_texts[index] = new_text;
    g_elements.texts.length++;
    g_elements.total_count++;

    return textId;
}

void Brick_LayoutText(Brick_ElementId textId) {
    // TODO: error handling
    if (textId.type != BRICK_ELEMENT_TYPE_TEXT) return;

    Brick_Text* text = Brick_Text_IndexGet(textId.index);

    CLAY_TEXT(text->clayString, BRICK_STYLE_TEXT_DEFAULT);
}

// Button
// _____________________________________________________________________________

bool Brick_IsButtonToggled(const Brick_ElementId buttonId) {
    Brick_ButtonState* state = Brick_ButtonState_Get(buttonId);

    return state->toggled;
}

void Brick_ToggleButton(Brick_ElementId buttonId) {
    Brick_ButtonState* state = Brick_ButtonState_Get(buttonId);

    state->toggled = !state->toggled;
}

void Brick_ToggleButton_Set(Brick_ElementId buttonId, bool isToggled) {
    Brick_ButtonState* state = Brick_ButtonState_Get(buttonId);

    state->toggled = isToggled;
}

Brick_ElementId Brick_CreateButton(const char* label) {
    Clay_String clayString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(label), 
        .chars = label 
    };

    int32_t index = g_elements.buttons.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_BUTTONS || g_elements.total_count >= BRICK_MAX_ELEMENTS) {
        return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_NONE);
    }

    Brick_ElementId buttonId = {
        .index = index,
        .type = BRICK_ELEMENT_TYPE_BUTTON,
    };

    Brick_Button new_button = {
        .clayId = CLAY_SID(clayString),
        .label = clayString,
        .id = buttonId,
        .imageData = nullptr,
        .state = Brick_ButtonState_DEFAULT,
        .groupIndex = 0,
        .width = 0,
        .height = 0,
        .fontSize = 0,
    };

    g_buttons[index] = new_button;
    g_elements.buttons.length++;
    g_elements.total_count++;

    return buttonId;
}

Brick_ElementId Brick_CreateToggleButton(const char* label) {
    Brick_ElementId buttonId = Brick_CreateButton(label);

    // TODO: error handling
    if (buttonId.type == BRICK_ELEMENT_TYPE_NONE) return buttonId;

    Brick_Button* button = Brick_Button_IndexGet(buttonId.index);

    Brick_ElementId toggleButtonId = PLEX(Brick_ElementId){ buttonId.index, BRICK_ELEMENT_TYPE_TOGGLE_BUTTON };
    button->id = toggleButtonId;
    
    return toggleButtonId;
}

// Button handlers
void Brick_OnHoverButtonState(Brick_ButtonState* state, int32_t idx, bool isHovering) {
    // Sets the following flags on the button:
    // hovered: the pointer is over the button (multiple frames)
    // cleared: the pointer has just stopped hovering (1 frame)

    // NOTE: gets called on every frame with every button
    // The button indexes are saved on the global context (g_window)
    // hoveredId: the current button being hovered
    // lastHoveredId: the last button that was hovered (after hovering on a new one) 
    if (isHovering) {
        // entering hover on button
        if (g_window.hoveredId != idx && g_window.lastHoveredId != idx) {
            g_window.hoveredId = idx;
            state->hovered = true;
        // one frame after entering hover
        } else if (g_window.hoveredId == idx && g_window.lastHoveredId != idx) {
            // propagate the cache to the last hover state
            g_window.lastHoveredId = idx;
        } 
    } else {
        // exiting hover
        if (g_window.hoveredId == idx) {
            g_window.hoveredId = 0;
            state->hovered = false;
            state->cleared = true;
        // one frame after exiting hover. Note: checking both last hover state, 
        // and the cleared flag for cases when pointer is moving really fast
        } else if (g_window.lastHoveredId == idx || state->cleared) {
            g_window.lastHoveredId = 0;
            state->cleared = false;
        }
    }
}

void Brick_HandleClayHoverButton(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Brick_Button* button = (Brick_Button*)userData;

    switch(pointerData.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        // if button is part of a group clear the toggled buttons
        if (button->groupIndex > 0) {
            Brick_ElementGroup* buttonGroup = Brick_ButtonGroup_IndexGet(button->groupIndex);
            for(int32_t j = 0; j < buttonGroup->length; j++) {
                int32_t buttonIdx = buttonGroup->ids[j];
                Brick_Button* groupButton = Brick_Button_IndexGet(buttonIdx);
                if (groupButton->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON){
                    groupButton->state.toggled = false;
                }
            }
        }
        button->state.clicked = true;
        button->state.toggled = !button->state.toggled;
    break;
    case CLAY_POINTER_DATA_PRESSED:
        button->state.clicked = false;
        button->state.pressed = true;
    break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        button->state.clicked = false;
        button->state.pressed = false;
        button->state.released = true;
    break;
    case CLAY_POINTER_DATA_RELEASED:
        // NOTE: This is almost the same as hover, Clay triggers this if pointer 
        // is on the button not pressing, and after pressing
    break;
    default: break;
    }
}

// internal button layout function using internal index
void Brick__LayoutButtonIndex(int32_t index) {
    Brick_Button* button = Brick_Button_IndexGet(index);
    Clay_Color bgColor = button->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON && button->state.toggled ? BRICK_COLOR_BUTTON_BG_TOGGLE : BRICK_COLOR_BUTTON_BG;
    Clay_Color borderColor = button->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON && button->state.toggled ? BRICK_COLOR_BUTTON_BORDER_TOGGLE : BRICK_COLOR_BUTTON_BORDER;

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
        // NOTE: Clay_Hovered only works inside the paramaters or declaration body
        .backgroundColor = Clay_PointerOver(button->clayId) ? BRICK_COLOR_BUTTON_BG_HOVER : bgColor,
        .border = { 
            .color = borderColor, 
            .width = CLAY_BORDER_OUTSIDE(1) 
        },
        // TODO: add to global styles
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    }) {
        Brick_OnHoverButtonState(&button->state, button->id.index, Clay_Hovered());
        // NOTE: Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHoverButton, button);
        CLAY_TEXT(button->label, BRICK_STYLE_BUTTON_LABEL);
    }
}

void Brick_LayoutButton(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_BUTTON && buttonId.type != BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) return;

    Brick__LayoutButtonIndex(buttonId.index);
}

// Image Button
// _____________________________________________________________________________

Brick_ElementId Brick_CreateImageButton(float width, float height, void* imageData) {
    int32_t index = g_elements.imageButtons.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_IMAGE_BUTTONS || g_elements.total_count >= BRICK_MAX_ELEMENTS) {
        return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_NONE);
    }

    Brick_ElementId buttonId = {
        .index = index,
        .type = BRICK_ELEMENT_TYPE_IMAGE_BUTTON,
    };

    Brick_ImageButton new_button = {
        .id = buttonId,
        .imageData = imageData,
        .state = Brick_ButtonState_DEFAULT,
        .groupIndex = 0,
        .width = width,
        .height = height,
    };

    g_image_buttons[index] = new_button;
    g_elements.imageButtons.length++;
    g_elements.total_count++;

    return buttonId;
}

// ImageButton hover handler
void Brick_HandleClayHoverState(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Brick_ButtonState* state = (Brick_ButtonState*)userData;

    switch(pointerData.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        state->clicked = true;
        state->toggled = !state->toggled;
    break;
    case CLAY_POINTER_DATA_PRESSED:
        state->clicked = false;
        state->pressed = true;
    break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        state->clicked = false;
        state->pressed = false;
        state->released = true;
    break;
    case CLAY_POINTER_DATA_RELEASED:
        // NOTE: This is almost the same as hover, Clay triggers this if pointer 
        // is on the button not pressing, and after pressing
    break;
    default: break;
    }
}

void Brick__LayoutImageButtonIndex(int32_t index) {
    Brick_ImageButton* button = Brick_ImageButton_IndexGet(index);

    CLAY_AUTO_ID({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIXED(button->width),
                .height = CLAY_SIZING_FIXED(button->height),
            },
        },
        .image = { .imageData = button->imageData }
    }) {
        Brick_OnHoverButtonState(&button->state, button->id.index, Clay_Hovered());
        // Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHoverState, &button->state);
    }
}

void Brick_LayoutImageButton(Brick_ElementId buttonId) {
    // TODO: add error handling
    if (buttonId.type != BRICK_ELEMENT_TYPE_IMAGE_BUTTON) return;

    Brick__LayoutImageButtonIndex(buttonId.index);
}

// Button Group
// _____________________________________________________________________________

Brick_ElementId Brick_CreateButtonGroup(const Brick_ElementId* buttonIds, int32_t groupSize) {

    // get the next index to store in button
    int32_t index = g_elements.buttonGroups.length;
    // TODO: add error handling
    // groups do not add element count 
    if (index >= BRICK_MAX_BUTTON_GROUPS) return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_NONE);
    // default group init
    Brick_ElementGroup group = CLAY__DEFAULT_STRUCT;

    // iterate over the button ids
    for (int32_t i = 0; i < groupSize; i++) {
        if (buttonIds[i].type != BRICK_ELEMENT_TYPE_BUTTON && buttonIds[i].type != BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) {
            // TODO: exit or handle error 
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
        }

        Brick_Button* button = Brick_Button_IndexGet(buttonIds[i].index);

        if (button->id.index == 0) {
            // TODO: exit or handle error 
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", buttonIds[i].index);
            return Brick_CreateElementId(0, BRICK_ELEMENT_TYPE_BUTTON_GROUP);
        }

        // cross reference the group
        button->groupIndex = index;
        if (i == 0 && button->id.type == BRICK_ELEMENT_TYPE_TOGGLE_BUTTON) {
            button->state.toggled = true;
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

// TODO: add Group_Get to consolidate error checking
void Brick_LayoutButtonGroup(Brick_ElementId groupId) {
    // TODO: add error handling
    if (groupId.type != BRICK_ELEMENT_TYPE_BUTTON_GROUP) return;

    const Brick_ElementGroup* buttonGroup = Brick_ButtonGroup_IndexGet(groupId.index);

    for (int32_t i = 0; i < buttonGroup->length; i++) {
        Brick__LayoutButtonIndex(buttonGroup->ids[i]);
    }
}

//                                Containers
// ------------------------------------.----------------------------------------
// Layout Containers
// BeginLayout<Element> and EndLayout<Element>
// TODO: abstract .transition as a style

Brick_ContainerId Brick_CreateContainerId(int32_t index, Brick_ContainerType type) {
    Brick_ContainerId id = { index, type };
    return id;
}

// Scroll Box
// _____________________________________________________________________________

Brick_ContainerId Brick_CreateScrollBox(void) {
    int32_t index = g_containers.scrollBoxes.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_SCROLLBOXES || g_containers.total_count >= BRICK_MAX_CONTAINERS) {
        return Brick_CreateContainerId(0, BRICK_CONTAINER_TYPE_NONE);
    }

    Brick_ContainerId containerId = {
        .index = index,
        .type = BRICK_CONTAINER_TYPE_SCROLLBOX,
    };

    char scrollBarIdLabel[12];
    snprintf(scrollBarIdLabel, sizeof(scrollBarIdLabel), "scrollBox%d", index);
    Clay_String scrollBarIdString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(scrollBarIdLabel), 
        .chars = scrollBarIdLabel 
    };

    char scrollBoxParentIdLabel[18];
    snprintf(scrollBoxParentIdLabel, sizeof(scrollBoxParentIdLabel), "scrollBoxParent%d", index);
    Clay_String scrollBoxParentIdString = CLAY__INIT(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(scrollBoxParentIdLabel), 
        .chars = scrollBoxParentIdLabel 
    };

    Brick_ScrollBox new_scroll_box = {
        .clayId = CLAY_SID(scrollBarIdString),
        .clayParentId = CLAY_SID(scrollBoxParentIdString),
        .id = containerId,
        .clickOrigin = PLEX(Clay_Vector2){ 0, 0 },
        .positionOrigin = PLEX(Clay_Vector2){ 0, 0 },
        .scrollY = 0,
        .isPrimaryDown = false
    };

    g_scroll_boxes[index] = new_scroll_box;
    g_containers.scrollBoxes.length++;
    g_containers.total_count++;

    return containerId;
}

void Brick_BeginScrollBox(Brick_ContainerId scrollBoxId) {
    if (scrollBoxId.type != BRICK_CONTAINER_TYPE_SCROLLBOX) return;

    Brick_ScrollBox* scrollBox = Brick_ScrollBox_IndexGet(scrollBoxId.index);
    Brick_ContainerStack_Push(scrollBox->id.index);

    Clay__OpenElementWithId(scrollBox->clayParentId);
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = { 
            .padding = CLAY_PADDING_ALL(32), 
            .childGap = 12, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .backgroundColor = BRICK_THEME_BACKGROUND,
        .clip = { 
            .vertical = true, 
            .childOffset = Clay_GetScrollOffset()
        },
        // TODO: abstract this as a style
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    });
}

void Brick_EndScrollBox(void) {
    int32_t scrollBoxIndex = Brick_ContainerStack_Pop();
    // TODO: error handling
    if (scrollBoxIndex < 0) return;

    Brick_ScrollBox* scrollBox = Brick_ScrollBox_IndexGet(scrollBoxIndex);

    Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(scrollBox->clayParentId);
    if (scrollContainerData.found && scrollContainerData.scrollContainerDimensions.height < scrollContainerData.contentDimensions.height) {
        CLAY(scrollBox->clayId, {
            .floating = {
                .offset = { 
                    .y = -(scrollContainerData.scrollPosition->y / scrollContainerData.contentDimensions.height) * scrollContainerData.scrollContainerDimensions.height 
                },
                .parentId = scrollBox->clayParentId.id,
                .zIndex = 2,
                .attachPoints = { 
                    .element = CLAY_ATTACH_POINT_RIGHT_TOP, 
                    .parent = CLAY_ATTACH_POINT_RIGHT_TOP 
                },
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
            }
        }) {
            CLAY_AUTO_ID({
                .layout = { 
                    .sizing = { 
                        CLAY_SIZING_FIXED(12), 
                        CLAY_SIZING_FIXED((scrollContainerData.scrollContainerDimensions.height / scrollContainerData.contentDimensions.height) * scrollContainerData.scrollContainerDimensions.height) 
                    }
                },
                // TODO: map the theme at the top of file instead of using it directly
                .backgroundColor = Clay_Hovered() || scrollBox->isPrimaryDown ? BRICK_THEME_SECONDARY : BRICK_THEME_TERTIARY,
            });
        }
    }
    Clay__CloseElement();
}

// Panel
// _____________________________________________________________________________

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
        // TODO: abstract this as a style
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    });
}

void Brick_EndPanel(void) {
    Clay__CloseElement();
}

// Floating Panel
// _____________________________________________________________________________
// TODO: figure out z-index configuration when there are multiple panels
// test different overlapping floating panels with no z-index and add
// FloatingPanelEx that takes a z-index param

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
        // TODO: abstract this as a style
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    });
}

void Brick_EndFloatingPanel(void) {
    Clay__CloseElement();
}

// Horizontal Stack
// _____________________________________________________________________________

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
        // TODO: abstract this as a style
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    });
}

void Brick_EndHorizontalStack(void) {
    Clay__CloseElement();
}

// Vertical Stack
// _____________________________________________________________________________

void Brick_BeginVerticalStack(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .height = CLAY_SIZING_GROW(0),
            },
            .childGap = BRICK_STYLE_PADDING_SMALL, 
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        // TODO: abstract this as a style
        .transition = {
            .handler = Clay_EaseOut,
            .duration = 0.3f,
            .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
            .enter = { .setInitialState = FadeSlide },
            // .exit = { .setFinalState = FadeSlide },
        }
    });
}

void Brick_EndVerticalStack(void) {
    Clay__CloseElement();
}

// Offset
// _____________________________________________________________________________
// TODO: check why transition was creating a black background
// when it was being transitioned from a Panel with a black background

void Brick_BeginOffset(float x, float y) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(CLAY__INIT(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_FIT(0),
                .height = CLAY_SIZING_FIT(0),
            },
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, 
        },
        .floating = { 
            .offset = {x, y}, 
            // .zIndex = 1, 
            .attachPoints = { 
                CLAY_ATTACH_POINT_CENTER_CENTER, 
                CLAY_ATTACH_POINT_CENTER_CENTER 
            }, 
            .attachTo = CLAY_ATTACH_TO_PARENT 
        },
        // TODO: abstract this as a style
        // .transition = {
        //     .handler = Clay_EaseOut,
        //     .duration = 0.3f,
        //     .properties = static_cast<Clay_TransitionProperty>(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
        //     .enter = { .setInitialState = FadeSlide },
        //     // .exit = { .setFinalState = FadeSlide },
        // }
    });
}

void Brick_EndOffset(void) {
    Clay__CloseElement();
}

// ------------------------------------.----------------------------------------
//                               ERROR HANDLING
// =============================================================================

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
