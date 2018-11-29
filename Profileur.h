#ifndef DEF_PROFILEUR
#define DEF_PROFILEUR

#include <ostream>
#include <chrono>
#include <type_traits>
#include <vector>
#include <thread>
#include <mutex>

template<typename Duration, typename FN>
Duration profile(FN fn) {
    auto before = std::chrono::high_resolution_clock::now();
    fn();
    auto after = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<Duration>(after - before);
}

struct ScopedProfiler {
    const char* name;
public:
    ScopedProfiler(const char* name) noexcept;
    ~ScopedProfiler();
};

#ifdef ENABLE_PROFILING
#define PROFILE_SCOPE(name) ScopedProfiler __scoped_profiler_ugly_name__(name)
#else
#define PROFILE_SCOPE(name)
#endif

class EventProfiler {
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = typename clock::time_point;
    enum EventTypes {
        Start,
        End,
        Instant
    };
    struct Event {
        EventTypes type;
        std::chrono::microseconds tm;
        std::thread::id tid;
        const char* name;

        Event(const char* name, EventTypes type, std::chrono::microseconds tp) noexcept
        : type{type}
        , tm{tp}
        , tid{std::this_thread::get_id()}
        , name{name}
        {}

        Event(const char* name, EventTypes type, std::chrono::microseconds tp, std::thread::id tid) noexcept
        : type{type}
        , tm{tp}
        , tid{tid}
        , name{name}
        {}

        friend std::ostream& operator<<(std::ostream& out, const Event& ev);
    };

private:
    std::vector<Event> events;
    time_point initial_tp;
    std::mutex m;

    EventProfiler();
    EventProfiler(const EventProfiler&) = delete;
    EventProfiler& operator=(const EventProfiler&) = delete;
public:
    static EventProfiler& instance();

    void register_start_event(const char* name);
    void register_end_event(const char* name);
    void register_instant_event(const char* name);

    friend std::ostream& operator<<(std::ostream& out, const EventProfiler& profiler);
};

#endif