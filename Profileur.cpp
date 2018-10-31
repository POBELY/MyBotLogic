#include "Profileur.h"

ScopedProfiler::ScopedProfiler(const char* name) noexcept 
: name{name} {
    EventProfiler::instance().register_start_event(name);
}

ScopedProfiler::~ScopedProfiler() {
    EventProfiler::instance().register_end_event(name);
}

std::ostream& operator<<(std::ostream& out, const EventProfiler::Event& ev) {
    out << "{"
        << "\"name\": \"" << ev.name << "\","
        << "\"ph\": \"";

    switch (ev.type) {
    case EventProfiler::EventTypes::Start:
        out << 'B';
        break;
    case EventProfiler::EventTypes::End:
        out << 'E';
        break;
    case EventProfiler::EventTypes::Instant:
        out << 'i';
        break;
    }

    out << "\",";
    out << "\"ts\": " << ev.tm.count() << ","
        << "\"pid\": 0,"
        << "\"tid\": 0"
        << "}";
    return out;
}

EventProfiler::EventProfiler()
: events{}
, initial_tp{clock::now() }{
    events.reserve(4096);
}

EventProfiler& EventProfiler::instance() {
    static EventProfiler instance_;

    return instance_;
}

void EventProfiler::register_start_event(const char* name) {
    events.emplace_back(name, EventTypes::Start, 
                        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - initial_tp));
}

void EventProfiler::register_end_event(const char* name) {
    events.emplace_back(name, EventTypes::End,
                        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - initial_tp));
}

void EventProfiler::register_instant_event(const char* name) {
    events.emplace_back(name, EventTypes::Instant,
                        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - initial_tp));
}

std::ostream& operator<<(std::ostream& out, const EventProfiler& profiler) {
    out << "{";

    out << "\"traceEvents\": [";

    if(!profiler.events.empty()) {
        for(std::size_t event_index = 0; event_index < profiler.events.size() - 1; ++event_index) {
            const EventProfiler::Event& ev = profiler.events[event_index];
            out << ev << ",";
        }
        out << profiler.events.back();
    }
    out << "],";
    out << "\"displayTimeUnit\": \"ns\"";

    out << "}";

    return out;
}