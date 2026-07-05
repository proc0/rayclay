#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_set>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::milliseconds;
using Trigger = std::function<void()>;
using TimerId = uint64_t;

typedef struct TimerEvent {
    TimerId id;
    TimePoint expiry;
    Trigger callback;
    // Priority queue is a Max-Heap by default, 
    // overload '>' for Min-Heap (earliest time first).
    bool operator>(const TimerEvent& other) const {
        return expiry > other.expiry;
    }
} TimerEvent;

class Timer {
    std::unordered_set<TimerId> runningIds;
    std::unordered_set<TimerId> stoppedIds;
    // Min-Heap to keep the soonest timer at the top
    std::priority_queue<TimerEvent, std::vector<TimerEvent>, std::greater<TimerEvent>> timers;
    
    TimerId nextId = 0;

public:
    void (Timer::*update)() = &Timer::updateUnit;

    bool isEmpty();
    bool isRunning(TimerId);
	
	TimerId schedule(int delay, Trigger callback);
    
    void stop(TimerId);
    void updateUnit();
	void updateTimer();
};