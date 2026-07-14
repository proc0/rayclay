#include "window.hpp"

#include "config.hpp"
#include "type.hpp"

#include "raylib.h"
#include "raymath.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void Window::load() {
    // initialize the screen size for Web
    update({ 
        .id = Event::Input::IDLE,
        .position = { 0.0f, 0.0f }
    });
}

void Window::enlist(Layer* listener) {
    listeners.push_back(listener);
}

float Window::adapt(float value) const {
    return value * unit;
}

Rectangle Window::center(Rectangle area) const {
    return {
        diagonal.x*0.5f - area.width*0.5f,
        diagonal.y*0.5f - area.height*0.5f,
        area.width,
        area.height
    };
}

Rectangle Window::center(Rectangle area, Vector2 offset) const {
    return {
        diagonal.x*0.5f - area.width*0.5f + offset.x,
        diagonal.y*0.5f - area.height*0.5f + offset.y,
        area.width,
        area.height
    };
}

float Window::convert(float value) const {
    return value * unit/SCREEN_UNIT;
}

#if __EMSCRIPTEN__
EM_JS(int, getWindowWidth, (), {
    return window.document.querySelector('canvas').clientWidth;
});

EM_JS(int, getWindowHeight, (), {
    return window.document.querySelector('canvas').clientHeight;
});
#endif

void Window::update(InputEvent input) {
    // fps = GetFPS();
    #if __EMSCRIPTEN__
        int newWidth = getWindowWidth();
        int newHeight = getWindowHeight();
    #else
        int newWidth = GetScreenWidth();
        int newHeight = GetScreenHeight();
    #endif

    if(newWidth != diagonal.x || newHeight != diagonal.y){
        if (!timer.isRunning(resizeTimerId)) {
            resize(newWidth, newHeight);
            for (auto* listener : listeners) {
                listener->resize(newWidth, newHeight);
            }
            // debounce resize event for Web
            resizeTimerId = timer.schedule(SCREEN_RESIZE_RATE, nullptr);
        }
    }

    // TODO: move middle mouse button to Input, refactor
    // zooming and panning
    Vector2 mouseWheel = GetMouseWheelMoveV();
    
    if (mouseWheel.y > 0) {
        zoom(true);
    } else if (mouseWheel.y < 0) {
        zoom(false);
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) { 
        isTracking = true;
    }

    if (isToggleTracking && input.id == Event::Input::PRIMARY) {
        isTracking = true;
    }

    if (isTracking) {
        track(input);
    }
}

void Window::resize(int newWidth, int newHeight) {
    // calculate ratio based on screen diagonal
    ratio = ROUND4(sqrtf(powf(newWidth, 2.0f) + powf(newHeight, 2.0f))/unitRatio);

    width = newWidth;
    height = newHeight;
    widthf = static_cast<float>(newWidth);
    heightf = static_cast<float>(newHeight);
    halfWidthf = widthf*0.5f;
    halfHeightf = heightf*0.5f;
    halfWidth = static_cast<int>(halfWidthf);
    halfHeight = static_cast<int>(halfHeightf);

    diagonal = { widthf, heightf };
    extent = { halfWidthf, halfHeightf };
    unit = SCREEN_UNIT*ratio + zoomUnit*ratio;

    TraceLog(LOG_INFO, "WINDOW resized %ix%i - UNIT: %f", newWidth, newHeight, unit);
}

float Window::scale(float value) const {
    return value*ratio;
}

Rectangle Window::scale(Rectangle area) const {
    // float ratio = x/static_cast<float>(SCREEN_WIDTH);
    return {
        area.x * ratio,
        area.y * ratio,
        area.width * ratio,
        area.height * ratio,
    };
}

Vector2 Window::scale(Vector2 point) const {
    // float ratio = x/static_cast<float>(SCREEN_WIDTH);
    return {
        point.x * ratio,
        point.y * ratio
    };
}

void Window::toggleTrack() {
    isToggleTracking = true;
}

void Window::track(InputEvent input) {
    // TODO: move middle mouse button to Input, refactor
    bool beginTrack = isToggleTracking ? input.id == Event::Input::PRIMARY : IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
    bool continueTrack = isToggleTracking ? input.id == Event::Input::PRIMARY_DOWN : IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
    bool endTrack = isToggleTracking ? input.id == Event::Input::PRIMARY_UP : IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE);

    if (beginTrack) {
        Vector2 origin = { extent.x + camera.offset.x, extent.y + camera.offset.y };
        originDelta = Vector2Subtract(origin, input.position);
    } else if (continueTrack) {
        camera.offset.x = input.position.x - extent.x + originDelta.x;
        camera.offset.y = input.position.y - extent.y + originDelta.y;
        offset.x = camera.offset.x;
        offset.y = camera.offset.y;
    } else if (endTrack) {
        originDelta = { 0.0f, 0.0f };
        // stop tracking
        isTracking = false;
        isToggleTracking = false;
        isDoneTracking = true;
    }
}

bool Window::tracking() const {
    return isToggleTracking || isTracking;
}

bool Window::doneTracking() {
    if (isDoneTracking) {
        isDoneTracking = false;
        return true;
    }
    return false;
}

void Window::zoom(bool increase) {
    float amount = 0.1f;
    if (increase) {
        unit += amount*ratio;
        zoomUnit += amount;
    } else {
        unit -= amount*ratio;
        zoomUnit -= amount;
    }
}
