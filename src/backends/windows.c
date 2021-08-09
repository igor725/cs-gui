#include <windows.h>
#pragma comment(lib, "user32")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HANDLE hCursor, hConsole, hInput, hSend, hList;

cs_str wndclass = "CServer Window Class";

#define BUFFER_SIZE 80 * 64
cs_char conbuff[BUFFER_SIZE];
cs_size buffpos = 0;

void Backend_CreateWindow(void) {
  HINSTANCE hInst = GetModuleHandle(NULL);

  WNDCLASS wc = {
    .lpfnWndProc = WindowProc,
    .lpszClassName = wndclass,
    .hInstance = hInst
  };

  RegisterClass(&wc);

  HWND hWnd = CreateWindowEx(
    0, wndclass, "Minecraft Classic server",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
    CW_USEDEFAULT, 854, 477,
    NULL, NULL, hInst, NULL
  );

  if(!hWnd) return;
  ShowWindow(hWnd, SW_SHOW);
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
    case WM_COMMAND:
      if(LOWORD(wParam) == 102 && HIWORD(wParam) == BN_CLICKED) {
        // TODO: Button click
        return TRUE;
      }
      return FALSE;
    case WM_PAINT:
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
      EndPaint(hWnd, &ps);
      return FALSE;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Backend_WindowLoop(cs_bool *notclosed) {
  MSG msg;

  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if(!*notclosed) break;
  }
}
