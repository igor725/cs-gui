#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "user32")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HANDLE hWnd_g, hCursor, hConsole, hInput, hSend, hList;
WNDCLASS wc = {
  .lpfnWndProc = WindowProc,
  .lpszClassName = "CServer Window Class"
};


#define BUFFER_SIZE 80 * 64
cs_char conbuff[BUFFER_SIZE];
cs_size buffpos = 0;

void Backend_CreateWindow(void) {
  RegisterClass(&wc);

  hWnd_g = CreateWindowEx(
    0, wc.lpszClassName,
    "Minecraft Classic server",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
    CW_USEDEFAULT, 854, 477,
    NULL, NULL, NULL, NULL
  );

  if(!hWnd_g) return;
  ShowWindow(hWnd_g, SW_SHOW);
}

static void AppendString(cs_str str) {
  if(buffpos + 160 >= BUFFER_SIZE) {
    cs_str fn = String_FirstChar(conbuff, '\n');
    cs_size offset = fn - conbuff + 1;

    cs_char temp;
    for(cs_uintptr t = 0; t < buffpos; t++) {
      temp = conbuff[t];
      conbuff[t] = conbuff[t + offset];
      conbuff[t + offset] = '\0';
    }
    buffpos -= offset;
  }
  buffpos += String_Copy(conbuff + buffpos, BUFFER_SIZE, str);
  SetWindowText(hConsole, conbuff);
  SendMessage(hConsole, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
  SendMessage(hConsole, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessage(hConsole, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0);
}

static void SetupWindow(HWND hWnd) {
  HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
  hCursor = LoadCursor(NULL, IDC_ARROW);

  hConsole = CreateWindowEx(
    0, "EDIT", NULL,
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
    ES_LEFT | ES_MULTILINE | ES_READONLY,
    0, 0, 674, 418, hWnd, (HMENU)100, hInst, NULL
  );

  hInput = CreateWindowEx(
    0, "EDIT", NULL,
    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
    0, 418, 754, 20, hWnd, (HMENU)101, hInst, NULL
  );

  hSend = CreateWindowEx(
    0, "BUTTON", "Send",
    WS_CHILD | WS_VISIBLE | WS_BORDER | BS_DEFPUSHBUTTON,
    754, 418, 100, 20, hWnd, (HMENU)102, hInst, NULL
  );

  hList = CreateWindowEx(
    0, "LISTBOX", NULL,
    WS_CHILD | WS_VISIBLE | WS_BORDER,
    674, 0, 180, 418, hWnd, (HMENU)103, hInst, NULL
  );
}

static void OpenPlayerContextMenu(HWND hWnd, cs_int32 x, cs_int32 y) {
  cs_char playername[64];
  LRESULT item = SendMessage(hList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
  if(item != LB_ERR) {
    SendMessage(hList, LB_GETTEXT, (WPARAM)item, (LPARAM)playername);
    HMENU hMenu = CreatePopupMenu();
    InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, playername);
    InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING, 1, "Kick");
    InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING, 2, "Ban");
    InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING, 3, "Make OP");
    InsertMenu(hMenu, 5, MF_BYPOSITION | MF_STRING, 4, "Info");
    TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hWnd_g, NULL);
  }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CREATE:
      SetupWindow(hWnd);
      return FALSE;
    case WM_DESTROY:
      PostQuitMessage(0);
      return FALSE;
    case WM_SETCURSOR:
      if((HWND)wParam == hWnd) {
        SetCursor(hCursor);
        return TRUE;
      } else break;
    case WM_CONTEXTMENU:
      if((HWND)wParam == hList)
        OpenPlayerContextMenu(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
    case WM_COMMAND:
      if(LOWORD(wParam) == 102 && HIWORD(wParam) == BN_CLICKED) {
        // TODO: Button click
        return TRUE;
      }
      return FALSE;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Backend_AddUser(cs_str name) {
  SendMessage(hList, LB_ADDSTRING, (WPARAM)0, (LPARAM)name);
}

void Backend_RemoveUser(cs_str name) {
  cs_char tmp[64];
  for(cs_int32 i = 0; i < SendMessage(hList, LB_GETCOUNT, 0, 0); i++) {
    LRESULT len = SendMessage(hList, LB_GETTEXT, (WPARAM)i, (LPARAM)tmp);
    if(len && String_CaselessCompare2(name, tmp, len)) {
      SendMessage(hList, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
      break;
    }
  }
}

void Backend_CloseWindow(void) {
  DestroyWindow(hWnd_g);
  UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void Backend_WindowLoop(void) {
  MSG msg;

  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}
