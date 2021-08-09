#ifndef BACKEND_H
#define BACKEND_H
void Backend_CreateWindow(void);
void Backend_AddUser(cs_str name);
void Backend_RemoveUser(cs_str name);
void Backend_WindowLoop(void);
void Backend_CloseWindow(void);
#endif
