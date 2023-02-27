#pragma once

#include "Macros.h"

#include <chrono>

namespace vox::utils
{
    class Timer
    {
    private:
        std::chrono::system_clock::time_point started_time_;

    public:
        FORCE_INLINE void Start()
        {
            this->started_time_ = std::chrono::system_clock::now();
        }

        FORCE_INLINE std::chrono::microseconds GetElapsedMicroSec() const
        {
            auto current_time_ = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(current_time_ - this->started_time_);
        }

        FORCE_INLINE std::chrono::microseconds ReStartAndGetElapsedMicroSec()
        {
            auto previous_time = this->started_time_;
            this->started_time_ = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(this->started_time_ - previous_time);
        }

        FORCE_INLINE void AddTimeMicroSec( std::chrono::microseconds microseconds )
        {
            this->started_time_ += microseconds;
        }
    };

}