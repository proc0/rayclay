#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_set>
#include <assert.h>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
// using Duration = std::chrono::nanoseconds;

using TimerId = uint64_t;

typedef struct TimerEvent {
    TimerId id;
    TimePoint expiry;
    std::function<void()> callback;
    // Priority queue is a Max-Heap by default, 
    // so we overload '>' to make it a Min-Heap (earliest time first).
    bool operator>(const TimerEvent& other) const {
        return expiry > other.expiry;
    }
} TimerEvent;

class Timer {
    // Min-Heap to keep the soonest timer at the top
    std::priority_queue<TimerEvent, std::vector<TimerEvent>, std::greater<TimerEvent>> timers;
    std::unordered_set<TimerId> runningTimers;
    std::unordered_set<TimerId> cancelledTimers;
    
    TimerId nextId = 0;

public:

	TimerId schedule(uint64_t delay, std::function<void()> callback) {
	    assert(delay > 0);

	    auto delayMs = std::chrono::milliseconds(delay);

	    auto id = nextId++;
	    auto expiry = Clock::now() + delayMs;

    	runningTimers.emplace(id);

    	if (callback == nullptr) {
    		callback = []{};
    	}

	    timers.push({id, expiry, callback});

	    update = &Timer::updateTimer;
	    
	    return id;
	}

    void cancel(TimerId id) {
    	cancelledTimers.emplace(id);
    }

    bool isRunning(TimerId id) {
	    auto cancelledId = cancelledTimers.find(id);
	    auto runningId = runningTimers.find(id);

	    return cancelledId != cancelledTimers.end() || runningId != runningTimers.end();
    }

    bool isEmpty() {
	    return cancelledTimers.empty() && runningTimers.empty();
    }

    void updateNull() {}

    void (Timer::*update)() = &Timer::updateNull;
	
	void updateTimer() {
	    auto now = Clock::now();

	    while (!timers.empty() && timers.top().expiry <= now) {
	        // 1. Get the timer from the top
	        TimerEvent event = timers.top();
	        timers.pop();

	        auto it = cancelledTimers.find(event.id);
            if (it != cancelledTimers.end()) {
            	cancelledTimers.erase(event.id);
                continue; 
            }

	        event.callback();
            runningTimers.erase(event.id);

            if (timers.empty()) {
            	update = &Timer::updateNull;
            }
	    }
	}
};