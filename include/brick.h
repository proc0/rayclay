/* 

+---------+
|  BRICK  |
+---------+ 
v0.1

UI Component library built with Clay

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

#include <stdio.h>
#include <string.h>

#if defined(__cplusplus)
    #define PLEX(type) type
#else
    #define PLEX(type) (type)
#endif

// ------------------------------------.----------------------------------------
//                                 SETTINGS
// =============================================================================

// Max Elements and (Stateful) Containers 
// _____________________________________________________________________________
// Determines the element and container type arrays max length
// Arrays are initialized as static global arrays
#define BRICK_MAX_TEXTS 128
// MAX_BUTTONS should not exceed GROUP_SIZE * GROUPS
#define BRICK_MAX_BUTTONS 128
#define BRICK_MAX_BUTTON_GROUPS 8
// IMAGE_BUTTONS cannot be grouped
#define BRICK_MAX_IMAGES 64
// total max number of elements
#define BRICK_MAX_ELEMENT_GROUP_SIZE 16
#define BRICK_MAX_ELEMENTS (BRICK_MAX_TEXTS + BRICK_MAX_BUTTONS + BRICK_MAX_IMAGES)

// total max number of groups
#define BRICK_MAX_GROUPS 8
#define BRICK_MAX_GROUP_SIZE 16
// total number of components
#define BRICK_MAX_COMPONENTS (BRICK_MAX_BUTTONS + BRICK_MAX_GROUPS)


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
#define BRICK_THEME_ACCENT      BRICK_COLOR_YELLOW

// Styles
// _____________________________________________________________________________
#define BRICK_STYLE_TEXT_DEFAULT    CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_PRIMARY, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_LEFT })
#define BRICK_STYLE_TEXT_HIGHLIGHT  CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_ACCENT, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_LEFT })
#define BRICK_STYLE_TEXT_CENTERED   CLAY_TEXT_CONFIG({ .textColor = BRICK_THEME_PRIMARY, .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT, .textAlignment = CLAY_TEXT_ALIGN_CENTER })

// Theme-Style Mapping 
// _____________________________________________________________________________
#define BRICK_STYLE_BUTTON_LABEL            BRICK_STYLE_TEXT_DEFAULT
#define BRICK_STYLE_BUTTON_LABEL_HIGHLIGHT  BRICK_STYLE_TEXT_HIGHLIGHT
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

// Global shared state storing window and pointer information.
typedef struct {
    float width;
    float height;
    int32_t hoveredId;
    int32_t lastHoveredId;
} Brick_Window;

// Transient frame state storing pointer frame data.
typedef struct {
    float x;
    float y;
    float scrollX;
    float scrollY;
    bool pressed;
    bool released;
} Brick_PointerData;

// Elements are the basic building blocks 
typedef CLAY_PACKED_ENUM {
    BRICK_ELEMENT_TYPE_NONE,
    BRICK_ELEMENT_TYPE_TEXT,
    BRICK_ELEMENT_TYPE_IMAGE,
} Brick_ElementType;

typedef struct Brick_ElementId {
    int32_t index;
    Brick_ElementType type;
} Brick_ElementId;

// Components include Brick elements and map to Clay elements
// Components also trigger events
typedef CLAY_PACKED_ENUM {
    BRICK_COMPONENT_TYPE_NONE,
    BRICK_COMPONENT_TYPE_LABEL,
    BRICK_COMPONENT_TYPE_BUTTON,
    BRICK_COMPONENT_TYPE_GROUP,
} Brick_ComponentType;

typedef CLAY_PACKED_ENUM {
    BRICK_COMPONENT_SUBTYPE_NONE,
    BRICK_COMPONENT_SUBTYPE_LABEL,
    BRICK_COMPONENT_SUBTYPE_TOGGLE,
    BRICK_COMPONENT_SUBTYPE_IMAGE,
} Brick_ComponentSubType;

typedef struct Brick_ComponentId {
    int32_t index;
    Brick_ComponentType type;
    Brick_ComponentSubType subType;
} Brick_ComponentId;

// TODO: check for subtype if it is a component id, and check for relevant upper bound
#define BRICK_ID_EQUALS(idA, idB) (idA.index == idB.index && idA.type == idB.type)
#define BRICK_ID_NOT_NULL(id) (id.index > 0 && id.index < BRICK_MAX_COMPONENTS && id.type > 0)

// Events
// _____________________________________________________________________________
// Events are triggered by components and have two kinds of duration. They can last
// a single frame, or multiple frames. Single frame events usually signal
// when something started or ended, while multi-frame events signal something is
// currently happening.

// WARN: CHECK MAX COUNT WHEN ADDING EVENTS
#define BRICK_MAX_EVENT_TYPES 10
typedef CLAY_PACKED_ENUM {
    BRICK_EVENT_TYPE_NONE,
    // Pointer enters hover
    // Duration: single frame
    BRICK_EVENT_TYPE_HOVER,
    // Pointer is hovering
    // Duration: multi-frame
    BRICK_EVENT_TYPE_HOVERING,
    // Pointer exits hover
    // Duration: single frame
    BRICK_EVENT_TYPE_CLEAR,
    // Element is pressed and 
    // released quickly (click event)
    BRICK_EVENT_TYPE_PRESS,
    // Element is being pressed
    // Duration: multi-frame
    BRICK_EVENT_TYPE_PRESSING,
    // Element stopped being pressed
    // Duration: single frame
    BRICK_EVENT_TYPE_RELEASE,
} Brick_EventType;

typedef struct Brick_Event {
    // the element triggering the event
    Brick_ComponentId componentId;
    // event array index reference
    int32_t index;
    // the type of event triggered
    Brick_EventType type;
} Brick_Event;

// Events array container to match Clay's way of exposing
// render commands. This can be iterated on the user's side.
typedef struct Brick_EventArray {
    int32_t length;
    Brick_Event* data;
} Brick_EventArray;

// Elements
// _____________________________________________________________________________
// There are two broad categories of elements, interactable and non-interactable.
// Elements like Button or Image are interactable, triggering events.
// Non-interactable elements like Text do not trigger any events.

typedef struct Brick_Text {
    Clay_Color color;
    Clay_String string;
    Brick_ElementId id;
    uint16_t fontId;
    uint16_t fontSize;
    Clay_TextAlignment align;
} Brick_Text;

typedef struct {
    Brick_ElementId id;
    void* imageData;
    // Brick_Interaction action;
    float width;
    float height;
} Brick_Image;

// Components
// _____________________________________________________________________________
// There are two broad categories of elements, interactable and non-interactable.
// Elements like Button or Image are interactable, triggering events.
// Non-interactable elements like Text do not trigger any events.

typedef struct Brick_Interaction {
    bool hovered;
    bool cleared;
    bool clicked;
    bool pressed;
    bool released;
    bool toggled;
} Brick_Interaction;

typedef struct Brick_Button {
    Clay_ElementId clayId;
    Clay_String label;
    Brick_Interaction action;
    Brick_ComponentId id;
    int32_t groupIndex;
} Brick_Button;

typedef struct Brick_Group {
    int32_t indexes[BRICK_MAX_GROUP_SIZE];
    Brick_ComponentId id;
    int32_t length;
} Brick_Group;

// Containers
// _____________________________________________________________________________
// Containers have an embedded scope struct style, with Begin and End prefixes
// for its layout functions. There are two categories of containers:
//
// Stateful containers: require calling its Brick_Create<Container> initializer,
// and then passing the ID returned by it to the opening Brick_Begin<Container>.
//
// Stateless containers: Do not need creation or saving IDs, and any parameters
// taken by the Begin tag are for frame-time configuration of the layout.

// Only stateful containers have a type because they have an ID
typedef CLAY_PACKED_ENUM {
    BRICK_CONTAINER_TYPE_NONE,
    BRICK_CONTAINER_TYPE_SCROLLBOX,
} Brick_ContainerType;

typedef struct Brick_ContainerId {
    int32_t index;
    Brick_ContainerType type;
} Brick_ContainerId;

// Stateful container types
typedef struct Brick_ScrollBox {
    Clay_ElementId clayId;
    Clay_ElementId clayParentId;
    Brick_ContainerId id;
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    float scrollY;
    bool isPrimaryDown;
} Brick_ScrollBox;

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
bool Brick_IsEventTriggeredById(Brick_EventType eventType, Brick_ComponentId componentId);
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

// NOTE: Unused internally
Clay_Vector2 Brick_GetComponentPosition(Brick_ComponentId id);

// Text
void Brick_InlineText(const char* text);
Brick_ElementId Brick_CreateText(const char* text);
void Brick_LayoutText(Brick_ElementId textId);

// Image Button
Brick_ElementId Brick_CreateImage(float width, float height, void* imageData);
void Brick_LayoutImage(Brick_ElementId buttonId);

// Button
bool Brick_IsButtonToggled(const Brick_ComponentId buttonId);
void Brick_ToggleButton(Brick_ComponentId buttonId);
void Brick_ToggleButton_Set(Brick_ComponentId buttonId, bool isToggled);
Brick_ComponentId Brick_CreateButton(const char* label);
// TODO: implement
// Brick_ComponentId Brick_CreateButtonEx(const char* label, int32_t width, int32_t height, int32_t fontSize, void* imageData);
Brick_ComponentId Brick_CreateLabelButton(const char* label);
Brick_ComponentId Brick_CreateToggleButton(const char* label);
void Brick_LayoutButton(Brick_ComponentId buttonId);
void Brick_LayoutLabelButton(Brick_ComponentId buttonId);
void Brick_LayoutToggleButton(Brick_ComponentId buttonId);

// Button Group
Brick_ComponentId Brick_CreateGroup(const Brick_ComponentId* componentIds, int32_t groupSize);
Brick_ComponentId Brick_CreateToggleGroup(const Brick_ComponentId* componentIds, int32_t groupSize);
void Brick_LayoutGroup(Brick_ComponentId groupId);

//                                Containers
// ------------------------------------.----------------------------------------
// Begin<Container> requires closing with End<Container>

// Stateful Containers
// _____________________________________________________________________________
// adds a global state instance, Create<Container> is required 

// Scroll Box
Brick_ContainerId Brick_CreateScrollBox(void);
void Brick_BeginScrollBox(Brick_ContainerId scrollBoxId);
void Brick_EndScrollBox(void);

// Stateless Containers
// _____________________________________________________________________________
// Does not require creation or ID management

// Panel
// A container that.. TODO: what is the defining feature of panel?
void Brick_BeginPanel(void);
void Brick_EndPanel(void);

// Floating Panel
// A container that floats on top of a layer
void Brick_BeginFloatingPanel(void);
void Brick_EndFloatingPanel(void);

// Horizontal Stack
// A container that aligns its children horizontally
// Left aligned by default
void Brick_BeginHorizontalStack(void);
void Brick_EndHorizontalStack(void);

// Vertical Stack
// A container that aligns its children vertically
void Brick_BeginVerticalStack(void);
void Brick_EndVerticalStack(void);

// Offset
// TODO: merge with FloatingPanelEx
void Brick_BeginOffset(float x, float y);
void Brick_EndOffset(void);

// Wrap
// A container that shrinks to its elements
void Brick_BeginWrapper(void);
void Brick_EndWrapper(void);

// Dropdown
// Render a floating wrapper container below the given element
// TODO: maybe make this stateful to avoid calculating position every frame?
void Brick_BeginDropdown(Brick_ComponentId id);
void Brick_EndDropdown(void);

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

typedef struct Brick_ImageArray {
    int32_t length;
    Brick_Image* data;
} Brick_ImageArray;

typedef struct Brick_Elements {
    int32_t total_count;
    Brick_TextArray texts;
    Brick_ImageArray images;
} Brick_Elements;

// Components
// _____________________________________________________________________________
typedef struct Brick_ButtonArray {
    int32_t length;
    Brick_Button* data;
} Brick_ButtonArray;

typedef struct Brick_GroupArray {
    int32_t length;
    Brick_Group* data;
} Brick_GroupArray;

typedef struct Brick_Components {
    int32_t total_count;
    Brick_ButtonArray buttons;
    Brick_GroupArray groups;
} Brick_Components;

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
// window context
static Brick_Window g_brick_window = CLAY__DEFAULT_STRUCT;

// Events
static int32_t g_brick_events_last_length = 0;
// event array passed back to user to handle events
static Brick_Event g_brick_events[BRICK_MAX_COMPONENTS];
// events snapshot array stores which events were triggered per frame
static bool g_is_events_snapshot_stale = false;
static bool g_brick_events_snapshot[BRICK_MAX_EVENT_TYPES] = CLAY__DEFAULT_STRUCT;

// Elements
static Brick_Text   g_brick_texts[BRICK_MAX_TEXTS];
static Brick_Image  g_brick_images[BRICK_MAX_IMAGES];

static Brick_Elements g_brick_elements = {
    .total_count = 0,
    .texts = {
        .length = 0,
        .data = g_brick_texts
    },
    .images = {
        .length = 0,
        .data = g_brick_images
    },
};

// Components
static Brick_Button g_brick_buttons[BRICK_MAX_BUTTONS];
static Brick_Group  g_brick_groups[BRICK_MAX_GROUPS];

static Brick_Components g_brick_components = {
    .total_count = 0,
    .buttons = {
        .length = 0,
        .data = g_brick_buttons
    },
    .groups = {
        .length = 0,
        .data = g_brick_groups
    },
};

// Stateful Containers
static int32_t g_brick_container_stack[BRICK_MAX_CONTAINERS];
static Brick_ScrollBox g_brick_scroll_boxes[BRICK_MAX_SCROLLBOXES];
static Brick_Containers g_brick_containers = {
    .total_count = 0,
    .stack = {
        .length = 0,
        .data = g_brick_container_stack
    },
    .scrollBoxes = {
        .length = 0,
        .data = g_brick_scroll_boxes
    },
};

// default global placeholders
Brick_Event Brick_Event_DEFAULT                 = CLAY__DEFAULT_STRUCT;

Brick_ElementId Brick_ElementId_DEFAULT         = CLAY__DEFAULT_STRUCT;
Brick_Text Brick_Text_DEFAULT                   = CLAY__DEFAULT_STRUCT;
Brick_Image Brick_Image_DEFAULT                 = CLAY__DEFAULT_STRUCT;

Brick_ComponentId Brick_ComponentId_DEFAULT     = CLAY__DEFAULT_STRUCT;
Brick_Interaction Brick_Interaction_DEFAULT     = CLAY__DEFAULT_STRUCT;
Brick_Button Brick_Button_DEFAULT               = CLAY__DEFAULT_STRUCT;
Brick_Group Brick_Group_DEFAULT                 = CLAY__DEFAULT_STRUCT;

Brick_ContainerId Brick_ContainerId_DEFAULT     = CLAY__DEFAULT_STRUCT;
Brick_ScrollBox Brick_ScrollBox_DEFAULT         = CLAY__DEFAULT_STRUCT;

//                               Array Getters
// ------------------------------------.----------------------------------------
Brick_Event* Brick_EventArray_Get(Brick_EventArray* array, int32_t index) {                                                    
    return index < array->length && index >= 0 ? &array->data[index] : &Brick_Event_DEFAULT;
}

Brick_Text* Brick_Text_IndexGet(int32_t index) {                                                    
    return index < g_brick_elements.texts.length && index >= 0 ? &g_brick_elements.texts.data[index] : &Brick_Text_DEFAULT;
}

Brick_Image* Brick_Image_IndexGet(int32_t index) {                                                    
    return index < g_brick_elements.images.length && index >= 0 ? &g_brick_elements.images.data[index] : &Brick_Image_DEFAULT;
}

Brick_Button* Brick_Button_IndexGet(int32_t index) {                                                    
    return index < g_brick_components.buttons.length && index >= 0 ? &g_brick_components.buttons.data[index] : &Brick_Button_DEFAULT;
}    

Brick_Group* Brick_Group_IndexGet(int32_t index) {                                                    
    return index < g_brick_components.groups.length && index >= 0 ? &g_brick_components.groups.data[index] : &Brick_Group_DEFAULT;
}

// NOTE: unused internally
// TODO: abstract to a general Get_Element
Brick_Button* Brick_Button_Get(Brick_ComponentId buttonId) {
    // TODO: add element subType and check against that
    if (buttonId.type != BRICK_COMPONENT_TYPE_BUTTON) return &Brick_Button_DEFAULT;

    return Brick_Button_IndexGet(buttonId.index);
}

Brick_Interaction* Brick_Interaction_Get(Brick_ComponentId buttonId) {
    Brick_Interaction* buttonState = &Brick_Interaction_DEFAULT;
    
    // TODO: add element subType and check against that
    if (buttonId.type == BRICK_COMPONENT_TYPE_BUTTON) {
        Brick_Button* button = Brick_Button_IndexGet(buttonId.index);
        return &button->action;
    } 

    // else if (buttonId.type == BRICK_ELEMENT_TYPE_IMAGE) {
    //     Brick_Image* button = Brick_Image_IndexGet(buttonId.index);
    //     return &button->action;
    // }

    return buttonState;
}

Brick_ScrollBox* Brick_ScrollBox_IndexGet(int32_t index) {
    return index < g_brick_containers.scrollBoxes.length && index >= 0 ? &g_brick_containers.scrollBoxes.data[index] : &Brick_ScrollBox_DEFAULT;
}


Clay_ElementId Brick_ClayId_Get(Brick_ComponentId id) {
    Clay_ElementId clayId = CLAY__DEFAULT_STRUCT;
    Brick_Button* element = NULL;

    //TODO: add the other types
    switch(id.type) {
    case BRICK_COMPONENT_TYPE_BUTTON:
        element = Brick_Button_IndexGet(id.index);
        clayId = element->clayId;
    break;
    default: break;
    }

    return clayId;
}
//                               Array Setters
// ------------------------------------.----------------------------------------
int32_t Brick_ContainerStack_Pop(void) {
    if (g_brick_containers.stack.length <= 0) return -1;

    g_brick_containers.stack.length--;
    return g_brick_containers.stack.data[g_brick_containers.stack.length];
}

void Brick_ContainerStack_Push(int32_t index) {
    g_brick_containers.stack.data[g_brick_containers.stack.length] = index;
    g_brick_containers.stack.length++;
}

//                         Private Forward Declarations
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
    g_brick_window.width = width;
    g_brick_window.height = height;

    // 1. Query minimum memory required for default element limits
    uint64_t memorySize = Clay_MinMemorySize();
    // 2. Allocate memory (malloc, stack, or custom allocator)
    void* memory = malloc(memorySize);
    // 3. Create arena [clay.h:2150-2158]
    g_clay_arena = Clay_CreateArenaWithCapacityAndMemory(memorySize, memory);
    // 4. Initialize Clay [clay.h:2186-2188]
    Clay_Initialize(g_clay_arena, PLEX(Clay_Dimensions){ width, height }, PLEX(Clay_ErrorHandler){ .errorHandlerFunction = Brick_HandleError, .userData = NULL });
    // 5. Set the MeasureText function along with pointer to fonts
    Clay_SetMeasureTextFunction(measureTextFunction, fontData);

    // seed button array and button group array at index 0 as unit values
    Brick_CreateButton("BRICK");
    // Brick_CreateImage(0, 0, NULL);
    // bypassing Brick_GroupButtons that checks 0 as invalid
    g_brick_components.groups.data[0] = Brick_Group_DEFAULT;
    g_brick_components.groups.length++;
}

void Brick_Resize(float width, float height) {
    g_brick_window.width = width;
    g_brick_window.height = height;
    Clay_SetLayoutDimensions(PLEX(Clay_Dimensions){ width, height });
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
    return g_brick_window.hoveredId != 0 && g_brick_window.lastHoveredId != g_brick_window.hoveredId;
}
// Global pointer hover clear check on any button. This is meant to be used in UpdateEvents.
// WARN: using this function by itself can be a race condition with the button HoverHandler
bool Brick_PointerJustCleared() {
    return g_brick_window.hoveredId == 0 && g_brick_window.lastHoveredId != 0;
}

bool Brick_IsEventTriggeredById(Brick_EventType eventType, Brick_ComponentId id) {
    // TODO: add some error handling
    if (eventType > BRICK_MAX_EVENT_TYPES) return false;
    
    for (int32_t i = 0; i < g_brick_events_last_length; i++) {
        if (g_brick_events[i].type == eventType && g_brick_events[i].componentId.type == id.type && g_brick_events[i].index == id.index) {
            return true;
        }
    }

    return false;
}

bool Brick_IsEventTriggered(Brick_EventType eventType) {
    // TODO: add some error handling
    if (eventType >= BRICK_MAX_EVENT_TYPES) return false;
    
    return g_brick_events_snapshot[eventType];
}

Brick_EventArray Brick_PollEvents(void) {
    Brick_EventArray events = {
        .length = g_brick_events_last_length,
        .data = g_brick_events
    };

    return events;
}

// Brick only function that will handle any potential updates of elements per frame
Brick_EventArray Brick_UpdateEvents(Brick_PointerData pointerData, float deltaTime) {

    Clay_SetPointerState(PLEX(Clay_Vector2){ .x = pointerData.x, .y = pointerData.y }, pointerData.pressed);
    
    Brick_EventArray events = {
        .length = 0,
        .data = g_brick_events
    };

    // clear the events snapshot
    if (g_is_events_snapshot_stale) {
        for (int32_t i = 0; i < BRICK_MAX_EVENT_TYPES; i++) {
            g_brick_events_snapshot[i] = false;
        }
    }

    // Button Events ------------------------------------
    // skip unit button at index 0
    for (int32_t i = 1; i < g_brick_components.buttons.length; i++) {
        Brick_Button* button = Brick_Button_IndexGet(i);
        Brick_Interaction* action = &button->action;

        if(action->clicked && !action->pressed) {
            // prevents event from firing after button is
            // not rendered, i.e. clicking to change panels
            action->clicked = false;
            g_brick_events[events.length] = PLEX(Brick_Event){
                .componentId = button->id,
                .index = i,
                .type = BRICK_EVENT_TYPE_PRESS
            };
            events.length++;

            g_brick_events_snapshot[BRICK_EVENT_TYPE_PRESS] = true;
            // simulate the hover clear on click in case the
            // click stops rendering the current button 
            // (prevents HOVERING event sticking, i.e. always showing hand cursor)
            g_brick_events_snapshot[BRICK_EVENT_TYPE_CLEAR] = true;
        } 
        else if (action->pressed) { 
            g_brick_events[events.length] = PLEX(Brick_Event){
                .componentId = button->id,
                .index = i,
                .type = BRICK_EVENT_TYPE_PRESSING
            };
            events.length++;

            g_brick_events_snapshot[BRICK_EVENT_TYPE_PRESSING] = true;
        }
        else if(action->released) {
            // prevents from firing after button is
            // blocked or not rendered, i.e. showing a popup window
            action->released = false;
            g_brick_events[events.length] = PLEX(Brick_Event){
                .componentId = button->id,
                .index = i,
                .type = BRICK_EVENT_TYPE_RELEASE
            };
            events.length++;

            g_brick_events_snapshot[BRICK_EVENT_TYPE_RELEASE] = true;
            // reverses the HOVER clear simulation on click (PRESS)
            // by setting HOVER back to true and allowing HOVER to trigger
            // again if the button is still being rendered in the layout
            g_brick_events_snapshot[BRICK_EVENT_TYPE_HOVER] = true;
        }
        else if(action->hovered) {
            Brick_EventType eventType = Brick_PointerJustHovered() ? BRICK_EVENT_TYPE_HOVER : BRICK_EVENT_TYPE_HOVERING;
            g_brick_events[events.length] = PLEX(Brick_Event){
                .componentId = button->id,
                .index = i,
                .type = eventType
            };
            events.length++;

            g_brick_events_snapshot[eventType] = true;
        }
        else if(action->cleared) {
            g_brick_events[events.length] = PLEX(Brick_Event){
                .componentId = button->id,
                .index = i,
                .type = BRICK_EVENT_TYPE_CLEAR
            };
            events.length++;

            g_brick_events_snapshot[BRICK_EVENT_TYPE_CLEAR] = true;
        }
    }

    // Image Events ------------------------------------
    // for (int32_t i = 1; i < g_brick_elements.images.length; i++) {
    //     Brick_Image* button = Brick_Image_IndexGet(i);
    //     Brick_Interaction* action = &button->action;

    //     if(action->clicked && !action->pressed) {
    //         // prevents event from firing after button is
    //         // not rendered, i.e. clicking to change panels
    //         action->clicked = false;
    //         g_brick_events[events.length] = PLEX(Brick_Event){
    //             .elementId = button->id,
    //             .index = i,
    //             .type = BRICK_EVENT_TYPE_PRESS
    //         };
    //         events.length++;

    //         g_brick_events_snapshot[BRICK_EVENT_TYPE_PRESS] = true;
    //     } 
    //     else if (action->pressed) { 
    //         g_brick_events[events.length] = PLEX(Brick_Event){
    //             .elementId = button->id,
    //             .index = i,
    //             .type = BRICK_EVENT_TYPE_PRESSING
    //         };
    //         events.length++;

    //         g_brick_events_snapshot[BRICK_EVENT_TYPE_PRESSING] = true;
    //     }
    //     else if(action->released) {
    //         // prevents from firing after button is
    //         // blocked or not rendered, i.e. showing a popup window
    //         action->released = false;
    //         g_brick_events[events.length] = PLEX(Brick_Event){
    //             .elementId = button->id,
    //             .index = i,
    //             .type = BRICK_EVENT_TYPE_RELEASE
    //         };
    //         events.length++;

    //         g_brick_events_snapshot[BRICK_EVENT_TYPE_RELEASE] = true;
    //     }
    //     else if(action->hovered) {
    //         Brick_EventType eventType = Brick_PointerJustHovered() ? BRICK_EVENT_TYPE_HOVER : BRICK_EVENT_TYPE_HOVERING;
    //         g_brick_events[events.length] = PLEX(Brick_Event){
    //             .elementId = button->id,
    //             .index = i,
    //             .type = eventType
    //         };
    //         events.length++;

    //         g_brick_events_snapshot[eventType] = true;
    //     }
    //     else if(action->cleared) {
    //         g_brick_events[events.length] = PLEX(Brick_Event){
    //             .elementId = button->id,
    //             .index = i,
    //             .type = BRICK_EVENT_TYPE_CLEAR
    //         };
    //         events.length++;

    //         g_brick_events_snapshot[BRICK_EVENT_TYPE_CLEAR] = true;
    //     }
    // }

    // ScrollBox Events ------------------------------------
    // update Clay scroll containers once for all scrollboxes
    if (g_brick_containers.scrollBoxes.length > 0) {
        Clay_UpdateScrollContainers(true, PLEX(Clay_Vector2){ pointerData.scrollX*2.0f, pointerData.scrollY*2.0f }, deltaTime);
    }

    for (int32_t i = 0; i < g_brick_containers.scrollBoxes.length; i++) {
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
                Clay_Vector2 ratio = PLEX(Clay_Vector2){
                    container.contentDimensions.width / container.scrollContainerDimensions.width,
                    container.contentDimensions.height / container.scrollContainerDimensions.height,
                };

                if (container.config.vertical) {
                    container.scrollPosition->y = scrollBox->positionOrigin.y + (scrollBox->clickOrigin.y - pointerData.y) * ratio.y;
                }
                
                if (container.config.horizontal) {
                    container.scrollPosition->x = scrollBox->positionOrigin.x + (scrollBox->clickOrigin.x - pointerData.x) * ratio.x;
                }
            }
        }
    }

    // Update metadata ------------------------------------
    // update the length cache for querying events
    g_brick_events_last_length = events.length;

    // flag the events snapshot for clear
    if (events.length) g_is_events_snapshot_stale = true;

    return events;
}

//                               Shared Functions
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

#define BRICK_TRANSITION_FADE_SLIDE { .handler = Clay_EaseOut, .duration = 0.3f, .properties = PLEX(Clay_TransitionProperty)(CLAY_TRANSITION_PROPERTY_BORDER_COLOR | CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR), .enter = { .setInitialState = FadeSlide }, }

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
    Clay_String clayString = PLEX(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(text), 
        .chars = text 
    };

    CLAY_TEXT(clayString, BRICK_STYLE_TEXT_DEFAULT);
}

Brick_ElementId Brick_CreateText(const char* text) {
    Clay_String clayString = PLEX(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(text), 
        .chars = text 
    };

    int32_t index = g_brick_elements.texts.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_TEXTS) return Brick_ElementId_DEFAULT;

    Brick_ElementId textId = {
        .index = index,
        .type = BRICK_ELEMENT_TYPE_TEXT,
    };

    Brick_Text new_text = {
        .color = BRICK_THEME_PRIMARY,
        .string = clayString,
        .id = textId,
        .fontId = 0,
        .fontSize = BRICK_STYLE_FONT_SIZE_DEFAULT,
        .align = CLAY_TEXT_ALIGN_LEFT
    };

    g_brick_elements.texts.data[index] = new_text;
    g_brick_elements.texts.length++;
    g_brick_elements.total_count++;

    return textId;
}

void Brick_LayoutText(Brick_ElementId textId) {
    // TODO: error handling
    if (textId.type != BRICK_ELEMENT_TYPE_TEXT) return;

    Brick_Text* text = Brick_Text_IndexGet(textId.index);

    CLAY_TEXT(text->string, { .textColor = text->color, .fontId = text->fontId, .fontSize = text->fontSize, .textAlignment = text->align });
}

// Image Button
// _____________________________________________________________________________

Brick_ElementId Brick_CreateImage(float width, float height, void* imageData) {
    int32_t index = g_brick_elements.images.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_IMAGES || g_brick_elements.total_count >= BRICK_MAX_ELEMENTS) {
        return Brick_ElementId_DEFAULT;
    }

    Brick_ElementId buttonId = {
        .index = index,
        .type = BRICK_ELEMENT_TYPE_IMAGE,
    };

    Brick_Image new_button = {
        .id = buttonId,
        .imageData = imageData,
        // .action = Brick_Interaction_DEFAULT,
        .width = width,
        .height = height,
    };

    g_brick_elements.images.data[index] = new_button;
    g_brick_elements.images.length++;
    g_brick_elements.total_count++;

    return buttonId;
}

void Brick_HandleClayHoverAction(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Brick_Interaction* action = (Brick_Interaction*)userData;

    switch(pointerData.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        action->clicked = true;
        action->toggled = !action->toggled;
    break;
    case CLAY_POINTER_DATA_PRESSED:
        action->clicked = false;
        action->pressed = true;
    break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        action->clicked = false;
        action->pressed = false;
        action->released = true;
    break;
    case CLAY_POINTER_DATA_RELEASED:
        // NOTE: This is almost the same as hover, Clay triggers this if pointer 
        // is on the button not pressing, and after pressing
    break;
    default: break;
    }
}

void Brick__LayoutImageIndex(int32_t index) {
    Brick_Image* image = Brick_Image_IndexGet(index);

    CLAY_AUTO_ID({
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIXED(image->width),
                .height = CLAY_SIZING_FIXED(image->height),
            },
        },
        .image = { .imageData = image->imageData }
    }) {}

    // {
    //     Brick_OnHoverInteraction(&image->action, image->id.index, Clay_Hovered());
    //     // Clay_OnHover also handles click events
    //     Clay_OnHover(Brick_HandleClayHoverAction, &image->action);
    // }
}

void Brick_LayoutImage(Brick_ElementId id) {
    // TODO: add error handling
    if (id.type != BRICK_ELEMENT_TYPE_IMAGE) return;

    Brick__LayoutImageIndex(id.index);
}

//                                Components
// ------------------------------------.----------------------------------------
// Create<Component> - initializes the component state and returns ID for layout
// Layout<Component> - called inside containers with Begin and End

Brick_ComponentId Brick_CreateComponentId(int32_t index, Brick_ComponentType type, Brick_ComponentSubType subType) {
    Brick_ComponentId id = { index, type, subType };
    return id;
}

// NOTE: Unused internally
Clay_Vector2 Brick_GetComponentPosition(Brick_ComponentId id) {
    Clay_ElementId clayId = Brick_ClayId_Get(id);
    Clay_ElementData elementData = Clay_GetElementData(clayId);

    return PLEX(Clay_Vector2){ elementData.boundingBox.x, elementData.boundingBox.y };
}

// Button
// _____________________________________________________________________________

bool Brick_IsButtonToggled(Brick_ComponentId buttonId) {
    Brick_Interaction* action = Brick_Interaction_Get(buttonId);

    return action->toggled;
}

void Brick_ToggleButton(Brick_ComponentId buttonId) {
    Brick_Interaction* action = Brick_Interaction_Get(buttonId);

    action->toggled = !action->toggled;
}

void Brick_ToggleButton_Set(Brick_ComponentId buttonId, bool isToggled) {
    Brick_Interaction* action = Brick_Interaction_Get(buttonId);

    action->toggled = isToggled;
}

Brick_ComponentId Brick_CreateButton(const char* label) {
    Clay_String clayString = PLEX(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(label), 
        .chars = label 
    };

    int32_t index = g_brick_components.buttons.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_BUTTONS || g_brick_components.total_count >= BRICK_MAX_ELEMENTS) {
        return Brick_ComponentId_DEFAULT;
    }

    Brick_ComponentId buttonId = {
        .index = index,
        .type = BRICK_COMPONENT_TYPE_BUTTON,
        .subType = BRICK_COMPONENT_SUBTYPE_NONE,
    };

    Brick_Button new_button = {
        .clayId = CLAY_SID(clayString),
        .label = clayString,
        .action = Brick_Interaction_DEFAULT,
        .id = buttonId,
        .groupIndex = 0,
    };

    g_brick_components.buttons.data[index] = new_button;
    g_brick_components.buttons.length++;
    g_brick_components.total_count++;

    return buttonId;
}

Brick_ComponentId Brick_CreateLabelButton(const char* label) {
    Brick_ComponentId buttonId = Brick_CreateButton(label);

    // TODO: error handling
    if (buttonId.type == BRICK_COMPONENT_TYPE_NONE) return buttonId;

    Brick_Button* button = Brick_Button_IndexGet(buttonId.index);

    Brick_ComponentId labelButtonId = PLEX(Brick_ComponentId){ buttonId.index, BRICK_COMPONENT_TYPE_BUTTON, BRICK_COMPONENT_SUBTYPE_LABEL };
    button->id = labelButtonId;
    
    return labelButtonId;
}

//TODO: review if TOggleButton is needed after ToggleGroup added
Brick_ComponentId Brick_CreateToggleButton(const char* label) {
    Brick_ComponentId buttonId = Brick_CreateButton(label);

    // TODO: error handling
    if (buttonId.type == BRICK_COMPONENT_TYPE_NONE) return buttonId;

    Brick_Button* button = Brick_Button_IndexGet(buttonId.index);

    Brick_ComponentId toggleButtonId = PLEX(Brick_ComponentId){ buttonId.index, BRICK_COMPONENT_TYPE_BUTTON, BRICK_COMPONENT_SUBTYPE_TOGGLE };
    button->id = toggleButtonId;
    
    return toggleButtonId;
}

// Button handlers
void Brick_OnHoverInteraction(Brick_Interaction* action, int32_t idx, bool isHovering) {
    // Sets the following flags on the button:
    // hovered: the pointer is over the button (multiple frames)
    // cleared: the pointer has just stopped hovering (1 frame)

    // NOTE: gets called on every frame with every button
    // The button indexes are saved on the global context (g_brick_window)
    // hoveredId: the current button being hovered
    // lastHoveredId: the last button that was hovered (after hovering on a new one) 
    if (isHovering) {
        // entering hover on button
        if (g_brick_window.hoveredId != idx && g_brick_window.lastHoveredId != idx) {
            g_brick_window.hoveredId = idx;
            action->hovered = true;
        // one frame after entering hover
        } else if (g_brick_window.hoveredId == idx && g_brick_window.lastHoveredId != idx) {
            // propagate the cache to the last hover action
            g_brick_window.lastHoveredId = idx;
        } 
    } else {
        // exiting hover
        if (g_brick_window.hoveredId == idx) {
            g_brick_window.hoveredId = 0;
            action->hovered = false;
            action->cleared = true;
        // one frame after exiting hover. Note: checking both last hover action, 
        // and the cleared flag for cases when pointer is moving really fast
        } else if (g_brick_window.lastHoveredId == idx || action->cleared) {
            g_brick_window.lastHoveredId = 0;
            action->cleared = false;
        }
    }
}

void Brick_HandleClayHoverButton(Clay_ElementId elementId, Clay_PointerData pointerData, void* userData) {
    Brick_Button* button = (Brick_Button*)userData;

    switch(pointerData.state) {
    case CLAY_POINTER_DATA_PRESSED_THIS_FRAME:
        // if button is part of a group clear the toggled buttons
        if (button->groupIndex > 0) {
            Brick_Group* buttonGroup = Brick_Group_IndexGet(button->groupIndex);

            if (buttonGroup->id.subType == BRICK_COMPONENT_SUBTYPE_TOGGLE) {                
                for(int32_t j = 0; j < buttonGroup->length; j++) {
                    int32_t buttonIdx = buttonGroup->indexes[j];
                    // TODO: handle other component types that are grouped? (or maybe its only buttons here)
                    Brick_Button* groupButton = Brick_Button_IndexGet(buttonIdx);
                    // TODO: review if this is a good idea, or all button types should toggle
                    // if (groupButton->id.type == BRICK_COMPONENT_TYPE_BUTTON && (groupButton->id.subType == BRICK_COMPONENT_SUBTYPE_TOGGLE || groupButton->id.subType == BRICK_COMPONENT_SUBTYPE_LABEL)){
                    groupButton->action.toggled = false;
                    // }
                }
            }
        }
        button->action.clicked = true;
        button->action.toggled = !button->action.toggled;
    break;
    case CLAY_POINTER_DATA_PRESSED:
        button->action.clicked = false;
        button->action.pressed = true;
    break;
    case CLAY_POINTER_DATA_RELEASED_THIS_FRAME:
        button->action.clicked = false;
        button->action.pressed = false;
        button->action.released = true;
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
    Clay_Color bgColor = button->id.subType == BRICK_COMPONENT_SUBTYPE_TOGGLE && button->action.toggled ? BRICK_COLOR_BUTTON_BG_TOGGLE : BRICK_COLOR_BUTTON_BG;
    Clay_Color borderColor = button->id.subType == BRICK_COMPONENT_SUBTYPE_TOGGLE && button->action.toggled ? BRICK_COLOR_BUTTON_BORDER_TOGGLE : BRICK_COLOR_BUTTON_BORDER;

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
        .backgroundColor = Clay_PointerOver(button->clayId) ? BRICK_COLOR_BUTTON_BG_HOVER : bgColor,
        .border = { 
            .color = borderColor, 
            .width = CLAY_BORDER_OUTSIDE(1) 
        },
        .transition = BRICK_TRANSITION_FADE_SLIDE
    }) {
        Brick_OnHoverInteraction(&button->action, button->id.index, Clay_Hovered());
        // NOTE: Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHoverButton, button);
        // update text style on hover
        if (Clay_Hovered()) {
            CLAY_TEXT(button->label, BRICK_STYLE_BUTTON_LABEL_HIGHLIGHT);
        } else {
            CLAY_TEXT(button->label, BRICK_STYLE_BUTTON_LABEL);
        }
    }
}

void Brick_LayoutButton(Brick_ComponentId buttonId) {
    // TODO: add error handling
    // TODO: add element subtype and check that instead
    if (buttonId.type != BRICK_COMPONENT_TYPE_BUTTON) return;

    Brick__LayoutButtonIndex(buttonId.index);
}

void Brick_LayoutToggleButton(Brick_ComponentId buttonId) {
    // TODO: add error handling
    // TODO: add element subtype and check that instead
    if (buttonId.subType != BRICK_COMPONENT_SUBTYPE_TOGGLE) return;

    Brick__LayoutButtonIndex(buttonId.index);
}

void Brick__LayoutLabelButtonIndex(int32_t index) {

    Brick_Button* labelButton = Brick_Button_IndexGet(index);

    CLAY(labelButton->clayId, {
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_FIT(0),
                .height = CLAY_SIZING_FIT(0),
            },
            .padding = {
                BRICK_STYLE_PADDING_SMALL,
                BRICK_STYLE_PADDING_SMALL,
                BRICK_STYLE_PADDING_SMALL,
                BRICK_STYLE_PADDING_SMALL
            },
            .childAlignment = { .x = CLAY_ALIGN_X_LEFT },
        }, 
        .transition = BRICK_TRANSITION_FADE_SLIDE
    }) {
        // hover state handling
        Brick_OnHoverInteraction(&labelButton->action, labelButton->id.index, Clay_Hovered());
        // NOTE: Clay_OnHover also handles click events
        Clay_OnHover(Brick_HandleClayHoverButton, labelButton);
        // update text style on hover
        if (Clay_Hovered()) {
            CLAY_TEXT(labelButton->label, BRICK_STYLE_BUTTON_LABEL_HIGHLIGHT);
        } else {
            CLAY_TEXT(labelButton->label, BRICK_STYLE_BUTTON_LABEL);
        }
    }
}

void Brick_LayoutLabelButton(Brick_ComponentId buttonId) {
    // TODO: add error handling
    if (buttonId.subType != BRICK_COMPONENT_SUBTYPE_LABEL) return;

    Brick__LayoutLabelButtonIndex(buttonId.index);
}


// Component Group
// _____________________________________________________________________________

Brick_ComponentId Brick_CreateGroup(const Brick_ComponentId* componentIds, int32_t groupSize) {

    // get the next index to store in button
    int32_t groupIndex = g_brick_components.groups.length;
    // TODO: add error handling
    // groups do not add element count 
    if (groupIndex >= BRICK_MAX_GROUPS || groupSize > BRICK_MAX_GROUP_SIZE) return Brick_ComponentId_DEFAULT;
    // default group init
    Brick_Group group = CLAY__DEFAULT_STRUCT;

    // iterate over the button ids
    for (int32_t i = 0; i < groupSize; i++) {
        // if (componentIds[i].type != BRICK_COMPONENT_TYPE_BUTTON && componentIds[i].type != BRICK_COMPONENT_TYPE_LABEL) {
        //     // TODO: exit or handle error 
        //     printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", componentIds[i].index);
        //     return Brick_ComponentId_DEFAULT;
        // }

        // TODO: switch case on type and use appropriate getter to abstract grouping to all component types
        Brick_Button* button = Brick_Button_IndexGet(componentIds[i].index);

        if (button->id.index == 0) {
            // TODO: exit or handle error 
            printf("Brick Error: Cannot create button group. Invalid button ID %d.\n", componentIds[i].index);
            return Brick_ComponentId_DEFAULT;
        }

        // cross reference the group
        button->groupIndex = groupIndex;
        // if (i == 0 && button->id.subType == BRICK_COMPONENT_SUBTYPE_TOGGLE) {
        //     button->action.toggled = true;
        // }
        // store the button id in the group
        group.indexes[i] = button->id.index;
        group.length++;
    }

    // if all buttons are valid, store the group
    g_brick_components.groups.data[groupIndex] = group;
    g_brick_components.groups.length++;

    return Brick_CreateComponentId(groupIndex, BRICK_COMPONENT_TYPE_GROUP, BRICK_COMPONENT_SUBTYPE_NONE);
}

Brick_ComponentId Brick_CreateToggleGroup(const Brick_ComponentId* componentIds, int32_t groupSize) {
    // TODO: ToggleGroup only works on Buttons, if other types are passed, promote to buttons

    Brick_ComponentId groupId = Brick_CreateGroup(componentIds, groupSize);

    // TODO: error handling
    if (groupId.type == BRICK_COMPONENT_TYPE_NONE) return groupId;

    Brick_Group* group = Brick_Group_IndexGet(groupId.index);

    Brick_ComponentId toggleGroupId = PLEX(Brick_ComponentId){ groupId.index, BRICK_COMPONENT_TYPE_GROUP, BRICK_COMPONENT_SUBTYPE_TOGGLE };
    group->id = toggleGroupId;

    if (componentIds[0].type == BRICK_COMPONENT_TYPE_BUTTON) {
        Brick_Button* button = Brick_Button_IndexGet(componentIds[0].index);
        button->action.toggled = true;
    }

    return toggleGroupId;
}


void Brick_LayoutGroup(Brick_ComponentId groupId) {
    // TODO: add error handling
    if (groupId.type != BRICK_COMPONENT_TYPE_GROUP) return;

    const Brick_Group* group = Brick_Group_IndexGet(groupId.index);

    for (int32_t i = 0; i < group->length; i++) {
        // TODO: handle the other component types to use the specific get
        Brick_Button* button = Brick_Button_IndexGet(group->indexes[i]);

        switch(button->id.subType) {
        case BRICK_COMPONENT_SUBTYPE_NONE:
            Brick__LayoutButtonIndex(button->id.index);
        break;
        case BRICK_COMPONENT_SUBTYPE_LABEL:
            Brick__LayoutLabelButtonIndex(button->id.index);
        break;
        case BRICK_COMPONENT_SUBTYPE_TOGGLE:
            Brick__LayoutButtonIndex(button->id.index);
        break;
        default: break;
        }
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
    int32_t index = g_brick_containers.scrollBoxes.length;
    // TODO: add error handling
    if (index >= BRICK_MAX_SCROLLBOXES || g_brick_containers.total_count >= BRICK_MAX_CONTAINERS) {
        return Brick_CreateContainerId(0, BRICK_CONTAINER_TYPE_NONE);
    }

    Brick_ContainerId containerId = {
        .index = index,
        .type = BRICK_CONTAINER_TYPE_SCROLLBOX,
    };

    char scrollBarIdLabel[21];
    snprintf(scrollBarIdLabel, sizeof(scrollBarIdLabel), "scrollBox%d", index);
    Clay_String scrollBarIdString = PLEX(Clay_String){ 
        .isStaticallyAllocated = true, 
        .length = (int32_t)strlen(scrollBarIdLabel), 
        .chars = scrollBarIdLabel 
    };

    char scrollBoxParentIdLabel[27];
    snprintf(scrollBoxParentIdLabel, sizeof(scrollBoxParentIdLabel), "scrollBoxParent%d", index);
    Clay_String scrollBoxParentIdString = PLEX(Clay_String){ 
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

    g_brick_containers.scrollBoxes.data[index] = new_scroll_box;
    g_brick_containers.scrollBoxes.length++;
    g_brick_containers.total_count++;

    return containerId;
}

void Brick_BeginScrollBox(Brick_ContainerId scrollBoxId) {
    if (scrollBoxId.type != BRICK_CONTAINER_TYPE_SCROLLBOX) return;

    Brick_ScrollBox* scrollBox = Brick_ScrollBox_IndexGet(scrollBoxId.index);
    Brick_ContainerStack_Push(scrollBox->id.index);

    Clay__OpenElementWithId(scrollBox->clayParentId);
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
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
        .transition = BRICK_TRANSITION_FADE_SLIDE
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
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0),
            },
            .padding = CLAY_PADDING_ALL(BRICK_STYLE_PADDING_SMALL), 
        },
        .backgroundColor = BRICK_THEME_BACKGROUND,
        .transition = BRICK_TRANSITION_FADE_SLIDE
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
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_PERCENT(0.5f),
                .height = CLAY_SIZING_PERCENT(0.5f),
            },
            .padding = CLAY_PADDING_ALL(BRICK_STYLE_PADDING_SMALL), 
            .childGap = BRICK_STYLE_PADDING_SMALL,
            .childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_TOP },
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
        .transition = BRICK_TRANSITION_FADE_SLIDE
    });
}

void Brick_EndFloatingPanel(void) {
    Clay__CloseElement();
}

// Horizontal Stack
// _____________________________________________________________________________

void Brick_BeginHorizontalStack(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(0),
            },
            .childGap = BRICK_STYLE_PADDING_SMALL, 
            .childAlignment = { .x = CLAY_ALIGN_X_LEFT }, 
            .layoutDirection = CLAY_LEFT_TO_RIGHT 
        },
        .transition = BRICK_TRANSITION_FADE_SLIDE
    });
}

void Brick_EndHorizontalStack(void) {
    Clay__CloseElement();
}

// Vertical Stack
// _____________________________________________________________________________

void Brick_BeginVerticalStack(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .height = CLAY_SIZING_GROW(0),
            },
            .childGap = BRICK_STYLE_PADDING_SMALL, 
            .childAlignment = { .y = CLAY_ALIGN_Y_TOP }, 
            .layoutDirection = CLAY_TOP_TO_BOTTOM 
        },
        .transition = BRICK_TRANSITION_FADE_SLIDE
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
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
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
    });
}

void Brick_EndOffset(void) {
    Clay__CloseElement();
}

// Wrapper
// _____________________________________________________________________________

void Brick_BeginWrapper(void) {
    Clay__OpenElement();
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_FIT(0),
                .height = CLAY_SIZING_FIT(0),
            },
            .padding = CLAY_PADDING_ALL(BRICK_STYLE_PADDING_SMALL), 
            .childGap = BRICK_STYLE_PADDING_SMALL,
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, 
        },
        .backgroundColor = BRICK_THEME_BACKGROUND,
    });
}

void Brick_EndWrapper(void) {
    Clay__CloseElement();
}

void Brick_BeginDropdown(Brick_ComponentId parentId) {
    Clay_ElementId parentClayId = Brick_ClayId_Get(parentId);
    Clay_ElementData parentData = Clay_GetElementData(parentClayId);

    Clay__OpenElement();
    Clay__ConfigureOpenElement(PLEX(Clay_ElementDeclaration) {
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_FIT(0),
                .height = CLAY_SIZING_FIT(0),
            },
        },
        .backgroundColor = BRICK_THEME_BACKGROUND,
        .floating = { 
            // TODO: save styles in elemenet state or abstract this to DropdownEx function as an option
            .offset = { -BRICK_STYLE_PADDING_SMALL, parentData.boundingBox.height + BRICK_STYLE_PADDING_SMALL }, 
            .parentId = parentClayId.id,
            // TODO: figure out systematic way to get z-index, maybe on a global level
            // or checking nested depth
            .zIndex = 3,
            .attachPoints = { 
                CLAY_ATTACH_POINT_LEFT_TOP, 
                CLAY_ATTACH_POINT_LEFT_TOP 
            }, 
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID
        },
    });
}

void Brick_EndDropdown(void) {
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
    //     Brick_Initialize(g_brick_window.width, g_brick_window.height);
    // } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
    //     Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    //     Brick_Initialize(g_brick_window.width, g_brick_window.height);
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
