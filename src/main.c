
#include <core.h>
#include <str.h>
#include <platform.h>
#include <client.h>
#include <server.h>
#include <event.h>
#include <log.h>
#include <plugin.h>
#include "backend.h"
#include "interface.h"

THREAD_FUNC(WindowThread) {
	(void)param;
	Backend_WindowLoop();
	Server_Active = false;
	return 0;
}

static void HandshakeEvent(onHandshakeDone *obj) {
	Backend_AddUser(Client_GetName(obj->client));
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

			if(*cc == '\0') {
				conbuff[buffpos++] = '\r';
				conbuff[buffpos++] = '\n';
				continue;
			}
		}

		conbuff[buffpos++] = *cc;
	}
	Backend_UpdateLog();
}

Plugin_SetVersion(1);

Event_DeclareBunch (events) {
	EVENT_BUNCH_ADD('v', EVT_ONHANDSHAKEDONE, HandshakeEvent)
	EVENT_BUNCH_ADD('v', EVT_ONDISCONNECT, DisconnectEvent)
	EVENT_BUNCH_ADD('v', EVT_ONLOG, LogEvent)

	EVENT_BUNCH_END
};

void Plugin_RecvInterface(cs_str iname, void *iptr, cs_size isize) {
	InterfaceReceiver(iname, iptr, isize);
}

cs_bool Plugin_Load(void) {
	Backend_PreLaunch();
	Thread_Create(WindowThread, NULL, true);
	LoadInterfaces();
	return Event_RegisterBunch(events);
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	Backend_CloseWindow();
	Event_UnregisterBunch(events);
	return true;
}
