#pragma once

// #include "units.hpp"
#include "macros.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "timer.hpp"

// #include <box2d/types.h>
#include <raylib.h>

class Window {
	Camera2D& camera;
	Timer& timer;
	TimerId resizeTimerId;

    Vector2 originDelta = { 0.0f, 0.0f };
    Vector2 diagonal = { SCREEN_WIDTH, SCREEN_HEIGHT };
    Vector2 extent = { SCREEN_WIDTH*0.5f, SCREEN_HEIGHT*0.5f };

    float const unitRatio = ROUND4(UNIT_RATIO);

	std::vector<Layer*> listeners;

public:
    Vector2 offset = { 0.0f, 0.0f };
	float unit = SCREEN_UNIT;
	float zoomUnit = 0.0f;
    float ratio = 1.0f;

    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    float widthf = static_cast<float>(SCREEN_WIDTH);
    float heightf = static_cast<float>(SCREEN_HEIGHT);
    float halfWidthf = widthf*0.5f;
    float halfHeightf = heightf*0.5f;
    int halfWidth = static_cast<int>(SCREEN_WIDTH*0.5f);
    int halfHeight = static_cast<int>(SCREEN_HEIGHT*0.5f);

	bool isTracking = false;
	bool isToggleTracking = false;
	bool isDoneTracking = false;

	Window(Camera2D& camera, Timer& timer) : camera(camera), timer(timer) {};
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
