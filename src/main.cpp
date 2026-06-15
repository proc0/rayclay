#include "app.hpp"

int main(void){
    App* app = new App();
    
    app->load();
    app->start();
#ifndef __EMSCRIPTEN__
    app->unload(0, 0, app);
#endif

    return 0;
}