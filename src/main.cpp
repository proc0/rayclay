#include "app.hpp"

int main(void){
    App* app = new App();
    
    app->load();
    app->start();
    app->unload(0, 0, app);

    delete app;

    return 0;
}

                                                                          
//                                                ┌──────┐                   
//                         ┌──────────────────────► Clay ◄──────────────┐    
//                         │                      └─▲─▲──┘              │    
//                         │        ┌───────────────┘ │                 │    
//                         │        │                 │                 │    
//                         │        │                 │                 │    
//                         │        │                 │                 │    
//                         │        │                 │                 │    
//                         │        │                 │                 │    
//                         │        │                 │                 │    
//                   ┌─────┼──┐     │    ┌────────┬───┘                 │    
//     ..............│ Layout │.....│..... Widget │. ..  . .            │    
//     .             └────▲.──┘     │    └───▲────┘        .            │    
//     .                  │..       │        │             .            │    
//     ..                 │ ........│.......................            │    
//     .                  │         │        │             .            │    
//     .                  │         │        │             .            │    
//     .                  │         │        │        ┌─      ─┐        │    
//     ..                 └─────────┼────────┴────────► Style  ┼────────┘    
//     ..                           │                 └───.────┘             
//     ....................................................                  
// ┌───.────┐                  ┌────┼────┐                                   
// │ Window │-  -   -  -   -  -│ Surface │                                   
// └───▲────┘                  └────▲────┘                                   
//     │                            │                                        
//     │                            │                                        
//     │                            │                                        
//     │                            │                                        
//     │                            │                                        
//     │                            │                                        
//     │         ┌───────┐          │                                        
//     └─────────┼  App  ┼──────────┘                                        
//               └───────┘                                                   
                                                                              
// LAB: In order to handle all the different changing units
// like padding, font size, widths, heights of UI,
// create an intermediate class that uses Window
// and holds all the values and can be easily configured
// This class could be called Space or Dimension or
// Screen? something that is handling responsiveness,
// dimensions. It would be similar to JS Twailwind
// that has small units of data in different types
// and can be accessed and combined easily, and these
// are member fields that are memoized and resize
// dynamically on window resize. 
// Together with a Layout, and Widget, and Window
// it would form a fully responsive, performant,
// and ergonomic API for a UI library
// ALong with the small units exposed with different types
// it could also store entire configurations for Clay

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
