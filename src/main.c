#include <core.h>
#include <platform.h>
#include <server.h>
#include <event.h>
#include "backend.h"

Thread GuiThread;

THREAD_FUNC(WindowThread) {
  (void)param;
  Backend_CreateWindow();
  Backend_WindowLoop();
  Server_Active = false;
  GuiThread = NULL;
  return 0;
}

void SpawnEvent(Client *client) {
  if(client->playerData->firstSpawn)
    Backend_AddUser(Client_GetName(client));
}

void DisconnectEvent(Client *client) {
  Backend_RemoveUser(Client_GetName(client));
}

void MessageEvent(onMessage *a) {
  Backend_AppendLog(Client_GetName(a->client));
  Backend_AppendLog(": ");
  Backend_AppendLog(a->message);
  Backend_AppendLog("\r\n");
  Backend_UpdateLog();
}

Plugin_SetVersion(1)
cs_bool Plugin_Load(void) {
  GuiThread = Thread_Create(WindowThread, NULL, false);
  Event_RegisterVoid(EVT_ONSPAWN, (evtVoidCallback)SpawnEvent);
  Event_RegisterVoid(EVT_ONDISCONNECT, (evtVoidCallback)DisconnectEvent);
  Event_RegisterVoid(EVT_ONMESSAGE, (evtVoidCallback)MessageEvent);
  return true;
}

cs_bool Plugin_Unload(void) {
  Backend_CloseWindow();
  return true;
}
