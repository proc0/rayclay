#include "menu.hpp"

#include "brick.h"

void Menu::load() {
	buttonId = Brick_CreateToggleButton("HELLO");
    buttonId2 = Brick_CreateToggleButton("HELLO 2");
    buttonId3 = Brick_CreateToggleButton("HELLO 3");
    Brick_ElementId buttonGroup1[3] = { buttonId, buttonId2, buttonId3 };
    buttonGroupId1 = Brick_CreateButtonGroup(buttonGroup1, 3);
}

void Menu::update() {

}

void Menu::layout() {
    Brick_BeginFloatingPanel();
        Brick_BeginHorizontalStack();
            Brick_LayoutButtonGroup(buttonGroupId1);
        Brick_EndHorizontalStack();

        if(Brick_IsButtonToggled(buttonId)) {
            Brick_BeginPanel();
                Brick_InlineText("TAB 1");
            Brick_EndPanel();
        } else if(Brick_IsButtonToggled(buttonId2)) {
            Brick_BeginPanel();
                Brick_InlineText("This is the second tab.");
            Brick_EndPanel();
        } else if(Brick_IsButtonToggled(buttonId3)) {
            Brick_BeginPanel();
                Brick_InlineText("3rd TAB!!");
            Brick_EndPanel();
        }
    Brick_EndFloatingPanel();
}

void Menu::render() const {
	
}

void Menu::unload() {
	
}