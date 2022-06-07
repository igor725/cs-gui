#include <core.h>
#include <plugin.h>
#include <platform.h>
#include <str.h>
#include <client.h>
#include "interface.h"
#include "cs-base/src/base_itf.h"

BaseItf *Base_Interface = NULL;
Mutex *Base_Mutex = NULL;

void LoadInterfaces(void) {
	Base_Mutex = Mutex_Create();
	Plugin_RequestInterface(Plugin_RecvInterface, BASE_ITF_NAME);
}

void InterfaceReceiver(cs_str iname, void *iptr, cs_size isize) {
	if(String_Compare(iname, BASE_ITF_NAME)) {
		if(Base_Interface) {
			Memory_Free(Base_Interface);
			Base_Interface = NULL;
		}
		if(isize > 0)
			Base_Interface = iptr;
	}
}

void BanPlayer(Client *client) {
	Mutex_Lock(Base_Mutex);
	if(Base_Interface) {
		if(Base_Interface->banUser(Client_GetName(client)))
			Client_Kick(client, "You are banned!");
	}
	Mutex_Unlock(Base_Mutex);
}

void TogglePlayerOP(Client *client) {
	Mutex_Lock(Base_Mutex);
	cs_str pname = Client_GetName(client);
	if(Base_Interface) {
		if(Base_Interface->isOperator(pname))
			Base_Interface->deopUser(pname);
		else
			Base_Interface->opUser(pname);
	} else
		Client_SetOP(client, !Client_IsOP(client));
	Mutex_Unlock(Base_Mutex);
}
