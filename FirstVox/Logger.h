#pragma once

#include <sstream>
#include "Vector.h"

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
        template<>
        Logger& operator<<( const vox::data::Vector4f& t )
        {
            const float* f = (const float*)&t;
            ostr << '[' << f[0] << ' ' << f[1] << ' ' << f[2] << ' ' << f[3] << ']' << ' ';
            return *this;
        }
        template<>
        Logger& operator<<( const vox::data::Vector4i& t )
        {
            const int* f = (const int*)&t;
            ostr << '[' << f[0] << ' ' << f[1] << ' ' << f[2] << ' ' << f[3] << ']' << ' ';
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