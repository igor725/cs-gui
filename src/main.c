#include <core.h>
#include <platform.h>
#include <server.h>
#include <event.h>
#include "backend.h"

THREAD_FUNC(WindowThread) {
  (void)param;
  Backend_CreateWindow();
  Backend_WindowLoop();
  Server_Active = false;
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
  Thread_Create(WindowThread, NULL, true);
  Event_RegisterVoid(EVT_ONSPAWN, (evtVoidCallback)SpawnEvent);
  Event_RegisterVoid(EVT_ONDISCONNECT, (evtVoidCallback)DisconnectEvent);
  Event_RegisterVoid(EVT_ONMESSAGE, (evtVoidCallback)MessageEvent);
  return true;
}

cs_bool Plugin_Unload(cs_bool force) {
  (void)force;
  Backend_CloseWindow();
  return true;
}
