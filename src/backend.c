#include <core.h>
#include <str.h>
#include <log.h>
#include <client.h>
#include <command.h>
#include <world.h>
#include "backend.h"

#if defined(WINDOWS)
#include "backends/windows.c"
#elif defined(UNIX)
#include "backends/linux.c"
#endif
