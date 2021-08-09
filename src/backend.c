#include <core.h>
#include <str.h>
#include <log.h>
#include <client.h>
#include <command.h>
#include <world.h>
#include "backend.h"

#define BUFFER_SIZE 80 * 64
cs_char conbuff[BUFFER_SIZE];
cs_size buffpos = 0;

#if defined(WINDOWS)
#include "backends/windows.c"
#elif defined(UNIX)
#include "backends/linux.c"
#endif

void Backend_AppendLog(cs_str str) {
  if(buffpos + 160 >= BUFFER_SIZE) {
    cs_str fn = String_FirstChar(conbuff, '\n');
    cs_size offset = fn - conbuff + 1;

    cs_char temp;
    for(cs_uintptr t = 0; t < buffpos; t++) {
      temp = conbuff[t];
      conbuff[t] = conbuff[t + offset];
      conbuff[t + offset] = '\0';
    }
    buffpos -= offset;
  }
  buffpos += String_Copy(conbuff + buffpos, BUFFER_SIZE, str);
}

void Backend_UpdateLog(void) {
  Backend_SetConsoleText(conbuff);
}
