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

static void CallCommand(cs_str cmd, cs_str args) {
  CommandCallData ccdata;
  cs_char out[MAX_CMD_OUT];
  cs_bool haveOutput = true;

  Command *opc = Command_GetByName(cmd);
  if(opc) {
    ccdata.caller = NULL;
    ccdata.command = opc;
    ccdata.args = args;
    ccdata.out = out;
    *out = '\0';
    haveOutput = opc->func(&ccdata);
  } else
    String_Copy(out, MAX_CMD_OUT, "Unknown command");

  if(haveOutput) {
    Log_Info(out);
    String_Append(out, MAX_CMD_OUT, "\r\n");
    Backend_AppendLog(out);
    Backend_UpdateLog();
  }
}

static void ExecuteUserCommand(void) {
  cs_char buffer[512];
  if(Backend_GetInputText(buffer, 512)) {
    Backend_SetInputText("");
    Backend_AppendLog("> ");
    Backend_AppendLog(buffer);
    Backend_AppendLog("\r\n");
    Backend_UpdateLog();
    cs_char *args = (cs_char *)String_FirstChar(buffer, ' ');
    if(args) *args++ = '\0';
    CallCommand(buffer, args);
  }
}

static void PrintPlayerInfo(Client *client) {
  cs_char buf[512];
  String_FormatBuf(
    buf, 512,
    "Info about %s\r\n"
    "  Client: %s\r\n"
    "  Is OP: %s\r\n"
    "  Ping: %d\r\n"
    "  IP: %d.%d.%d.%d\r\n"
    "  World: %s\r\n"
    "  Player model: %d\r\n",
    Client_GetName(client),
    Client_GetAppName(client),
    Client_IsOP(client) ? "yes" : "no",
    Client_GetPing(client),
    client->addr & 0xFF,
    (client->addr >> 8) & 0xFF,
    (client->addr >> 16) & 0xFF,
    (client->addr >> 24) & 0xFF,
    World_GetName(Client_GetWorld(client)),
    Client_GetModel(client)
  );
  Backend_AppendLog(buf);
  Backend_UpdateLog();
}

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
