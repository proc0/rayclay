#pragma once

// #include "units.hpp"
#include "macros.hpp"
#include "types.hpp"
#include "defaults.hpp"

// #include <box2d/types.h>
#include <raylib.h>
#include <chrono>

//TODO: refactor to use timer instead
//TODO: provide a way to use float or int width or height
//TODO: refactor width/height to be easier to use, make it public, figure out if getter/setter is needed or not

class Window {
	Camera2D& camera;
    Vector2 originDelta = { 0.0f, 0.0f };
    float const unitRatio = ROUND4(UNIT_RATIO);
	int x;
	int y;
	// resize debounce, prevents too many calls for Web
    std::chrono::milliseconds const timeResizeRate = std::chrono::milliseconds(SCREEN_RESIZE_RATE);
    std::chrono::steady_clock::time_point timeLastResize = std::chrono::steady_clock::now();
	int halfX;
	int halfY;

public:
    Vector2 offset = { 0.0f, 0.0f };
	float unit;
	float zoomUnit = 0.0f;
    float ratio = 1.0f;
	// int fps;
	int width() const;
	int height() const;
	int halfWidth() const;
	int halfHeight() const;
	bool isTracking = false;
	bool isToggleTracking = false;
	bool isDoneTracking = false;

	std::vector<Layer*> listeners;

	Window(Camera2D& camera):
		camera(camera),
		x(SCREEN_WIDTH),
		y(SCREEN_HEIGHT),
		halfX(SCREEN_WIDTH/2),
		halfY(SCREEN_HEIGHT/2),
		unit(SCREEN_UNIT) {};
	// fps(TARGET_FPS) {};
	~Window() = default;

	void load();
	// float adapt(float value) const;
	// Vector2 adapt(b2Vec2 area) const;
	// float convert(float value) const;
	// Vector2 convert(b2Vec2 area) const;
	// Vector2 project(b2Vec2 point) const;
	// Rectangle project(Rectangle area) const;
	// b2Vec2 inject(Vector2 point) const;
	void enlist(Layer* listener);
	void resize(int newWidth, int newHeight);
	float scale(float value) const;
	Rectangle scale(Rectangle area) const;
	Vector2 scale(Vector2 vect) const;
	Rectangle center(Rectangle area) const;
	Rectangle center(Rectangle area, Vector2 offset) const;
	void toggleTrack();
	void track(InputEvent);
	bool tracking() const;
	bool doneTracking();
	void update(InputEvent);
	void zoom(bool increase);
};
