#pragma once
#include <chrono>

namespace vox::util
{
    class Timer
    {
    public:
        inline void Start();
        inline std::chrono::microseconds GetElapsedMicroSec() const;
        inline std::chrono::microseconds ReStartAndGetElapsedMicroSec();
        inline void AddTimeMicroSec( std::chrono::microseconds microseconds );
    private:
        std::chrono::system_clock::time_point started_time_;
    };

    inline void Timer::Start()
    {
        started_time_ = std::chrono::system_clock::now();
    }
    inline std::chrono::microseconds Timer::GetElapsedMicroSec() const
    {
        auto current_time_ = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(current_time_ - started_time_);
    }
    inline std::chrono::microseconds Timer::ReStartAndGetElapsedMicroSec()
    {
        auto previous_time = started_time_;
        started_time_ = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(started_time_ - previous_time);
    }
    inline void Timer::AddTimeMicroSec( std::chrono::microseconds microseconds )
    {
        started_time_ += microseconds;
    }
}