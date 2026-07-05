#include "timer.hpp"

#include <assert.h>

TimerId Timer::schedule(uint64_t delay, std::function<void()> callback) {
    assert(delay > 0);

    auto id = nextId++;
    auto expiry = Clock::now() + Duration(delay);

	runningIds.emplace(id);

	if (callback == nullptr) {
		callback = []{};
	}

    timers.push({ id, expiry, callback });

    update = &Timer::updateTimer;

    return id;
}

void Timer::stop(TimerId id) {
	stoppedIds.emplace(id);
}

bool Timer::isRunning(TimerId id) {
    auto runningId = runningIds.find(id);
    return runningId != runningIds.end();
}

bool Timer::isEmpty() {
    return stoppedIds.empty() && runningIds.empty();
}

void Timer::updateNull() {}

void Timer::updateTimer() {
    auto now = Clock::now();

    while (!timers.empty() && timers.top().expiry <= now) {

        TimerEvent event = timers.top();
        timers.pop();

        // check if timer was stopped
        auto result = stoppedIds.find(event.id);
        if (result != stoppedIds.end()) {
        	stoppedIds.erase(event.id);
            continue; 
        }

        event.callback();
        runningIds.erase(event.id);

        if (timers.empty()) {
        	update = &Timer::updateNull;
        }
    }
}
