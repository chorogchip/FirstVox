#include "Logger.h"

#include <Windows.h>

namespace vox::logger
{

    void Logger::LogDebugString()
    {
        this->ostr << '\n';
        OutputDebugStringA( this->ostr.str().c_str() );
        this->ostr.str( "" );
    }
}