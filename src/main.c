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

void HandshakeEvent(Client *client) {
  Backend_AddUser(Client_GetName(client));
}

void DisconnectEvent(Client *client) {
  Backend_RemoveUser(Client_GetName(client));
}

Plugin_SetVersion(1)
cs_bool Plugin_Load(void) {
  GuiThread = Thread_Create(WindowThread, NULL, false);
  Event_RegisterVoid(EVT_ONHANDSHAKEDONE, (evtVoidCallback)HandshakeEvent);
  Event_RegisterVoid(EVT_ONDISCONNECT, (evtVoidCallback)DisconnectEvent);

  return true;
}

cs_bool Plugin_Unload(void) {
  Backend_CloseWindow();
  return true;
}
