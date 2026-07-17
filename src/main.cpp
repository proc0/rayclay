#include "app.hpp"

int main(void){
    App* app = new App();
    
    app->load();
    app->start();
    app->unload(0, 0, app);

    delete app;

    return 0;
}

// LAB: For menus, Surface should have one layout method
// with a shell Clay element that calls the different 
// layout layers within Surface (i.e. layoutMainMenu, or
// layoutPauseMenu). This would allow transition effects
// to work when changing UI screens. It might need
// refactoring of Layout member function pointers and
// how App is referencing them. Watchout for perf issues.

// NOTE: This could be a vector/stack of Layouts/Layers that
// extend and abstract class, and Surface would loop
// through the layers calling their methods, removing and
// adding Layouts/Layers as needed. Each Layout would use
// Widget to render its components glueing them together.

// LAB: Breakup Surface into Display and Layout classes,
// or just break up Surface into multiple files. Using
// different classes could allow composition between
// Layout, Widget, and Display, with Surface containing
// the methods equivalent for Pages or Screens.

// TODO: Add support for multiple scrollbars by enhancing
// how Surface and Widget handle them

// LAB: Look into transition Clay_Text to allow for better 
// transitioning effects with elements containing text.
