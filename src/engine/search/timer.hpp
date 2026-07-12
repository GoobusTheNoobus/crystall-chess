#pragma once

#include "chess/types.hpp"
#include <chrono>
#include <atomic>

namespace Crystall {
    using Time = std::chrono::steady_clock::time_point;

    struct Timer {
        private:
        inline static Time start_time;
        inline static int max_time_ms;
        inline static std::atomic_bool stop_flag;

        public:
        inline static void start(int max_time) {
            max_time_ms = max_time;
            start_time = std::chrono::steady_clock::now();
            stop_flag.store(false);
        }

        static void request_stop() {
            stop_flag.store(true);
        }

        inline static int elapsed() {
            return std::chrono::duration_cast<std::chrono::milliseconds>
                    (std::chrono::steady_clock::now() - start_time).count();
        }

        inline static bool should_stop_search() {
            return stop_flag.load() || (max_time_ms > 0 && elapsed() > max_time_ms);
        }
    };
}

