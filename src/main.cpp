#include "app.hpp"

int main(void){
    App* app = new App();
    
    app->load();
    app->start();
    app->unload(0, 0, app);

    delete app;

    return 0;
}

// TODO: Add better example of Tutorial screen, maybe one dummy image
// TODO: add example of background scrolling
// TODO: add example of button image
// TODO: finish other todos, cleanup Surface and Widget
// TODO: add slightly more demo game to test other states
