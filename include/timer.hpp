#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_set>
#include <unordered_map>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Millisecs = std::chrono::milliseconds;
using Secs = std::chrono::seconds;
using Duration = std::chrono::duration<double>;
using Trigger = std::function<void()>;
using TimerId = uint64_t;

struct TimerEvent {
    TimerId id;
    TimePoint expiry;
    Trigger callback;
    // priority queue is max-heap by default, 
    // overload '>' for min-heap (earliest time first)
    bool operator>(const TimerEvent& other) const {
        return expiry > other.expiry;
    }
};

class Timer {
    std::unordered_set<TimerId> runningIds;
    std::unordered_set<TimerId> stoppedIds;
    // min-heap to keep the soonest timer at the top
    std::priority_queue<TimerEvent, std::vector<TimerEvent>, std::greater<TimerEvent>> timers;
    std::unordered_map<TimerId, std::pair<TimePoint, Millisecs>> timepoints;

    TimerId nextId = 0;

public:
    void (Timer::*update)() = &Timer::updateUnit;

    bool isEmpty();
    bool isRunning(TimerId);
	
	TimerId schedule(int delay, Trigger callback);
    TimerId startWatch();
    void stopWatch(TimerId);
    std::string getWatchTime(TimerId);
    std::string consumeWatchTime(TimerId);
    void eraseWatchTime(TimerId);

    void stop(TimerId);
    void updateUnit();
	void updateTimer();
};