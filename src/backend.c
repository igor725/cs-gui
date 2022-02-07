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
cs_bool scrolltoend = true;
cs_size endpos = 0;

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

#define YESNO(exp) (exp ? "yes" : "no")

static void PrintPlayerInfo(Client *client) {
  cs_char buf[512];
  cs_uint32 addr = Client_GetAddr(client);
  String_FormatBuf(
    buf, 512,
    "Info about %s\r\n"
    "  Client: %s\r\n"
    "  Is OP: %s\r\n"
    "  Is in game: %s\r\n"
    "  Ping: %d\r\n"
    "  IP: %d.%d.%d.%d\r\n"
    "  World: %s\r\n"
    "  Player model: %d\r\n",
    Client_GetName(client),
    Client_GetAppName(client),
    YESNO(Client_IsOP(client)),
    YESNO(Client_CheckState(client, PLAYER_STATE_INGAME)),
    Client_GetPing(client),
    addr & 0xFF,
    (addr >> 8) & 0xFF,
    (addr >> 16) & 0xFF,
    (addr >> 24) & 0xFF,
    World_GetName(Client_GetWorld(client)),
    Client_GetModel(client)
  );
  Backend_AppendLog(buf);
  Backend_UpdateLog();
}

#if defined(CORE_USE_WINDOWS)
#include "backends/windows.c"
#elif defined(CORE_USE_UNIX)
#include "backends/linux.c"
#endif

void Backend_AppendLog(cs_str str) {
  cs_size slen = String_Length(str);
  if(buffpos + slen >= BUFFER_SIZE) {
    cs_size shiftend = BUFFER_SIZE - slen;
    for(cs_uintptr i = 0; i < shiftend; i++) {
      conbuff[i] = conbuff[i + slen];
      conbuff[i + slen] = '\0';
    }
    buffpos -= slen;
  }
  buffpos += String_Copy(conbuff + buffpos, BUFFER_SIZE - buffpos, str);
}

void Backend_UpdateLog(void) {
  if(Backend_GetScrollPosition() != endpos) {
    scrolltoend = false;
  }
  Backend_SetConsoleText(conbuff);
  if(scrolltoend)
    endpos = Backend_ScrollToEnd();
  else
    endpos = Backend_GetScrollEnd();
}
