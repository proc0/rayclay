#include "timer.hpp"

#include <assert.h>
#include <chrono>

bool Timer::isRunning(TimerId id) {
    auto runningId = runningIds.find(id);
    return runningId != runningIds.end();
}

bool Timer::isEmpty() {
    return stoppedIds.empty() && runningIds.empty();
}

TimerId Timer::startWatch() {
    TimerId id = nextId++;
    timepoints[id] = std::make_pair(Clock::now(), Millisecs(0));

    return id;
}

void Timer::stopWatch(TimerId id) {
    auto result = timepoints.find(id);
    if (result != timepoints.end()) {
        auto& timePair = result->second;
        TimePoint start = timePair.first;
        TimePoint end = Clock::now();
        
        auto totalTime = end - start;
        auto totalSecs = std::chrono::duration_cast<Millisecs>(totalTime);

        timePair.second = totalSecs;
    }
}

std::string Timer::getWatchTime(TimerId id) {
    auto result = timepoints.find(id);
    
    std::string value = "";
    if (result != timepoints.end()) {
        auto& timePair = result->second;
        value = std::format("{:%M:%S}", std::chrono::duration_cast<Secs>(timePair.second));
    }

    return value;
}

std::string Timer::consumeWatchTime(TimerId id) {
    std::string totalSecs = getWatchTime(id);
    if (totalSecs.length()) {
        eraseWatchTime(id);
    }
    return totalSecs;
}

void Timer::eraseWatchTime(TimerId id) {
    timepoints.erase(id);
}

TimerId Timer::schedule(int delay, Trigger callback) {
    assert(delay > 0);

    auto id = nextId++;
    auto expiry = Clock::now() + Millisecs(delay);

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

void Timer::updateUnit() {}

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
        	update = &Timer::updateUnit;
        }
    }
}
