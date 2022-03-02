#include <core.h>
#include <str.h>
#include <platform.h>
#include <client.h>
#include <server.h>
#include <event.h>
#include <log.h>
#include "backend.h"
#include <plugin.h>

THREAD_FUNC(WindowThread) {
	(void)param;
	Backend_WindowLoop();
	Server_Active = false;
	return 0;
}

static void SpawnEvent(void *param) {
	onSpawn *a = (onSpawn *)param;
	if(Client_IsFirstSpawn(a->client))
		Backend_AddUser(Client_GetName(a->client));
}

static void DisconnectEvent(Client *client) {
	Backend_RemoveUser(Client_GetName(client));
}

static cs_str escend = "HfABCDsuJKmhlp";

static void LogEvent(void *a) {
	LogBuffer *buf = (LogBuffer *)a;
	Backend_ShiftBuffer(String_Length(buf->data));
	for(cs_char *cc = buf->data; *cc != '\0' && buffpos < BUFFER_SIZE; cc++) {
		if(*cc == '\x1B') {
			cs_bool isInEscape = true;
			while(*++cc != '\0' && isInEscape)
				for(cs_str ce = escend; *ce != '\0' && isInEscape; ce++)
					if(*ce == *cc) isInEscape = false;
		}

		conbuff[buffpos++] = *cc;
	}
	Backend_UpdateLog();
}

Plugin_SetVersion(1);

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_ONSPAWN, SpawnEvent)
	EVENT_BUNCH_ADD('v', EVT_ONDISCONNECT, DisconnectEvent)
	EVENT_BUNCH_ADD('v', EVT_ONLOG, LogEvent)

	EVENT_BUNCH_END
};

cs_bool Plugin_Load(void) {
	Backend_PreLaunch();
	Thread_Create(WindowThread, NULL, true);
	return Event_RegisterBunch(events);
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	Backend_CloseWindow();
	Event_UnregisterBunch(events);
	return true;
}
