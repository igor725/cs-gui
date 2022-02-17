#include <core.h>
#include <platform.h>
#include <server.h>
#include <event.h>
#include <log.h>
#include "backend.h"
#include <plugin.h>

THREAD_FUNC(WindowThread) {
	(void)param;
	Backend_CreateWindow();
	Backend_WindowLoop();
	Server_Active = false;
	return 0;
}

void SpawnEvent(void *param) {
	onSpawn *a = (onSpawn *)param;
	if(Client_IsFirstSpawn(a->client))
		Backend_AddUser(Client_GetName(a->client));
}

void DisconnectEvent(Client *client) {
	Backend_RemoveUser(Client_GetName(client));
}

void OnLog(void *a) {
	LogBuffer *buf = (LogBuffer *)a;
	Backend_AppendLog(buf->data);
	Backend_UpdateLog();
}

Plugin_SetVersion(1)
cs_bool Plugin_Load(void) {
	Thread_Create(WindowThread, NULL, true);
	Event_RegisterVoid(EVT_ONSPAWN, (evtVoidCallback)SpawnEvent);
	Event_RegisterVoid(EVT_ONDISCONNECT, (evtVoidCallback)DisconnectEvent);
	Event_RegisterVoid(EVT_ONLOG, (evtVoidCallback)OnLog);
	return true;
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	Backend_CloseWindow();
	return true;
}
