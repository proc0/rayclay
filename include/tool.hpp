#pragma once

#include <cmath> // IWYU pragma: export

#define INTERSECTS(a, b) ((a.x > b.x) && (a.x < b.x + b.width) && (a.y > b.y) && (a.y < b.y + b.height))

#define ROUND4(num) (round((num) * 10000.0f)/10000.0f)
