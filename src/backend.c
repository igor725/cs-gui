#include <core.h>
#include <str.h>
#include <log.h>
#include "backend.h"

#if defined(WINDOWS)
#include "backends/windows.c"
#elif defined(UNIX)
#include "backends/linux.c"
#endif
