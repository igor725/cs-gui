#ifndef CSGUI_INTERFACE_H
#define CSGUI_INTERFACE_H
#include <core.h>

void LoadInterfaces(void);
void InterfaceReceiver(cs_str iname, void *iptr, cs_size isize);

void BanPlayer(Client *client);
void TogglePlayerOP(Client *client);
#endif
