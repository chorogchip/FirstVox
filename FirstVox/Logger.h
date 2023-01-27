#pragma once

#include <sstream>

namespace vox::logger
{
    struct LoggerFlusher { };

    class Logger
    {
    private:
        std::stringstream ostr;

    public:
        Logger() : ostr{} { }

        template<typename T>
        Logger& operator<<( const T& t )
        {
            ostr << t << ' ';
            return *this;
        }
        template<>
        Logger& operator<<( const LoggerFlusher& t )
        {
            this->LogDebugString();
            return *this;
        }
        std::string GetString()
        {
            return ostr.str();
        }
        void LogDebugString();
    };

    inline Logger GLogger;
    inline LoggerFlusher end;
}