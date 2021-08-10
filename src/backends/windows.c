#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "user32")

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK subInputProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HANDLE hDefaultCursor;
struct {
  HANDLE hWnd, hConsole, hInput, hSend, hList;
  WNDPROC lpfnInputProc;
  WNDCLASS wc;
} mainCTX = {
  .wc = {
    .lpfnWndProc = WindowProc,
    .lpszClassName = "CServer Window Class"
  }
};

static void OpenPlayerContextMenu(HWND hWnd, cs_int32 x, cs_int32 y) {
  cs_char playername[64];
  LRESULT item = SendMessage(mainCTX.hList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
  if(item != LB_ERR) {
    SendMessage(mainCTX.hList, LB_GETTEXT, (WPARAM)item, (LPARAM)playername);
    Client *client = Client_GetByName(playername);
    if(client) {
      HMENU hMenu = CreatePopupMenu();
      InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, playername);
      InsertMenu(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
      InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING, 1, "Kick");
      InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING, 2, "Ban");
      if(Client_IsOP(client))
        InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING, 3, "Revoke OP");
      else
        InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING, 3, "Make OP");
      InsertMenu(hMenu, 5, MF_BYPOSITION | MF_STRING, 4, "Info");

      switch (TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hWnd, NULL)) {
        case 0: break;

        case 1: // Kick
          CallCommand("kick", playername);
          break;
        case 2: // Ban
          CallCommand("ban", playername);
          break;
        case 3: // Make OP
          CallCommand(Client_IsOP(client) ? "deop" : "makeop", playername);
          break;
        case 4: // Info
          PrintPlayerInfo(client);
          break;
      }
      DestroyMenu(hMenu);
    }
  }
}

static void SetupWindow(HWND hWnd) {
  HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
  hDefaultCursor = LoadCursor(NULL, IDC_ARROW);

  mainCTX.hConsole = CreateWindowEx(
    0, "EDIT", NULL,
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
    ES_LEFT | ES_MULTILINE | ES_READONLY,
    0, 0, 674, 418, hWnd, (HMENU)100, hInst, NULL
  );

  mainCTX.hInput = CreateWindowEx(
    0, "EDIT", NULL,
    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
    0, 418, 754, 20, hWnd, (HMENU)101, hInst, NULL
  );
  mainCTX.lpfnInputProc = (WNDPROC)SetWindowLongPtr(mainCTX.hInput, GWLP_WNDPROC, (LONG_PTR)subInputProc);

  mainCTX.hSend = CreateWindowEx(
    0, "BUTTON", "Send",
    WS_CHILD | WS_VISIBLE | WS_BORDER | BS_DEFPUSHBUTTON,
    754, 418, 100, 20, hWnd, (HMENU)102, hInst, NULL
  );

  mainCTX.hList = CreateWindowEx(
    0, "LISTBOX", NULL,
    WS_CHILD | WS_VISIBLE | WS_BORDER,
    674, 0, 180, 418, hWnd, (HMENU)103, hInst, NULL
  );
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
        SetCursor(hDefaultCursor);
        return TRUE;
      } else break;
    case WM_CONTEXTMENU:
      if((HWND)wParam == mainCTX.hList)
        OpenPlayerContextMenu(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
    case WM_COMMAND:
      if(LOWORD(wParam) == 102 && HIWORD(wParam) == BN_CLICKED) {
        ExecuteUserCommand();
        return TRUE;
      }
      return FALSE;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK subInputProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_KEYUP:
      switch(wParam) {
        case VK_RETURN:
          ExecuteUserCommand();
          return 0;
      }
    default:
      return CallWindowProc(mainCTX.lpfnInputProc, hWnd, uMsg, wParam, lParam);
  }
}

void Backend_CreateWindow(void) {
  RegisterClass(&mainCTX.wc);

  if((mainCTX.hWnd = CreateWindow(
    mainCTX.wc.lpszClassName,
    "Minecraft Classic server",
    WS_OVERLAPPEDWINDOW &
    ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
    CW_USEDEFAULT, CW_USEDEFAULT,
    854, 477, NULL, NULL, NULL, NULL
  )) == NULL) return;

  ShowWindow(mainCTX.hWnd, SW_SHOW);
}

void Backend_AddUser(cs_str name) {
  SendMessage(mainCTX.hList, LB_ADDSTRING, (WPARAM)0, (LPARAM)name);
}

void Backend_RemoveUser(cs_str name) {
  cs_char tmp[64];
  for(cs_int32 i = 0; i < SendMessage(mainCTX.hList, LB_GETCOUNT, 0, 0); i++) {
    LRESULT len = SendMessage(mainCTX.hList, LB_GETTEXT, (WPARAM)i, (LPARAM)tmp);
    if(len && String_CaselessCompare2(name, tmp, len)) {
      SendMessage(mainCTX.hList, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
      break;
    }
  }
}

void Backend_CloseWindow(void) {
  DestroyWindow(mainCTX.hWnd);
  UnregisterClass(mainCTX.wc.lpszClassName, mainCTX.wc.hInstance);
}

void Backend_SetConsoleText(cs_str txt) {
  SetWindowText(mainCTX.hConsole, txt);
  SendMessage(mainCTX.hConsole, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
  SendMessage(mainCTX.hConsole, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
  SendMessage(mainCTX.hConsole, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0);
}

cs_size Backend_GetInputText(cs_char *buff, cs_size len) {
  return SendMessage(mainCTX.hInput, WM_GETTEXT, (WPARAM)len, (LPARAM)buff);
}

cs_bool Backend_SetInputText(cs_str txt) {
  return (cs_bool)SendMessage(mainCTX.hInput, WM_SETTEXT, (WPARAM)0, (LPARAM)txt);
}

void Backend_WindowLoop(void) {
  MSG msg;

  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}
