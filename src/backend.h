#ifndef BACKEND_H
#define BACKEND_H
#include <core.h>

#define BUFFER_SIZE 80 * 64
extern cs_char conbuff[BUFFER_SIZE];
extern cs_size buffpos;

void Backend_CreateWindow(void);
void Backend_AddUser(cs_str name);
void Backend_RemoveUser(cs_str name);
void Backend_CloseWindow(void);
void Backend_SetConsoleText(cs_str txt);
cs_size Backend_ScrollToEnd(void);
cs_size Backend_GetScrollPosition(void);
cs_size Backend_GetScrollEnd(void);
cs_size Backend_GetInputText(cs_char *buff, cs_size len);
cs_bool Backend_SetInputText(cs_str txt);
void Backend_WindowLoop(void);

// Shared between backends
void Backend_ShiftBuffer(cs_size slen);
void Backend_AppendLog(cs_str str);
void Backend_UpdateLog(void);
#endif
