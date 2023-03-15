#pragma once

#define TO_USE_CRTDBG 0

#if TO_USE_CRTDBG
#define _CRTDBG_MAP_ALLOC
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__ ) 
#endif

