#pragma once

#if defined (DEBUG) || defined (_DEBUG)
#define M_DEBUG
#endif

#define VEC_CALL __vectorcall
#define FORCE_INLINE __forceinline
#define FALL_THROUGH __fallthrough

#define M_STRINGIZE_DETAIL(x) #x
#define M_STRINGIZE(x) M_STRINGIZE_DETAIL(x)
#define M_LOGERROR(msg) do OutputDebugStringA("error logged in file " __FILE__ " line " M_STRINGIZE(__LINE__) ": " msg "\n" ); while (0)

