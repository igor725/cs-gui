#include <core.h>
#include <platform.h>
#include <server.h>
#include "backend.h"

Thread GuiThread;

THREAD_FUNC(WindowThread) {
  (void)param;
  Backend_CreateWindow();
  Backend_WindowLoop(&Server_Active);
  Server_Active = false;
  GuiThread = NULL;
  return 0;
}

Plugin_SetVersion(1)
cs_bool Plugin_Load(void) {
  GuiThread = Thread_Create(WindowThread, NULL, false);
  return true;
}

cs_bool Plugin_Unload(void) {
  if(Thread_IsValid(GuiThread))
    Thread_Join(GuiThread);
  return true;
}
