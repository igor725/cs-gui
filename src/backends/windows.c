#include <core.h>
#include <client.h>
#include <windows.h>
#include <windowsx.h>
#include "backend.h"
#include "interface.h"
#pragma comment(lib, "user32")

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK subInputProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HANDLE hDefaultCursor;
static struct {
	HANDLE hWnd, hOutput, hInput, hSend, hList;
	WNDPROC lpfnInputProc;
	const WNDCLASS wc;
} mainCTX = {
	.wc = {
		.lpfnWndProc = WindowProc,
		.lpszClassName = "CServer Window Class"
	}
};

static void OpenPlayerContextMenu(HWND hWnd, cs_int32 x, cs_int32 y) {
	cs_char playername[64];
	LRESULT item = SendMessageA(mainCTX.hList, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if(item != LB_ERR) {
		SendMessageA(mainCTX.hList, LB_GETTEXT, (WPARAM)item, (LPARAM)playername);
		Client *client = Client_GetByName(playername);
		if(client) {
			HMENU hMenu = CreatePopupMenu();
			InsertMenuA(hMenu, 0, MF_BYPOSITION | MF_STRING | MF_GRAYED, 0, playername);
			InsertMenuA(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenuA(hMenu, 2, MF_BYPOSITION | MF_STRING, 1, "Kick");
			InsertMenuA(hMenu, 3, MF_BYPOSITION | MF_STRING, 2, "Ban");
			if(Client_IsOP(client))
				InsertMenuA(hMenu, 4, MF_BYPOSITION | MF_STRING, 3, "Revoke OP");
			else
				InsertMenuA(hMenu, 4, MF_BYPOSITION | MF_STRING, 3, "Make OP");
			InsertMenuA(hMenu, 5, MF_BYPOSITION | MF_STRING, 4, "Info");

			switch (TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hWnd, NULL)) {
				case 0: break;

				case 1: // Kick
					Client_Kick(client, NULL);
					break;
				case 2: // Ban
					BanPlayer(client);
					break;
				case 3: // Make OP
					TogglePlayerOP(client);
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
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrA(hWnd, GWLP_HINSTANCE);
	hDefaultCursor = LoadCursorA(NULL, IDC_ARROW);

	mainCTX.hOutput = CreateWindowA(
		"EDIT", NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
		ES_LEFT | ES_MULTILINE | ES_READONLY,
		0, 0, 674, 418, hWnd, (HMENU)100, hInst, NULL
	);

	mainCTX.hInput = CreateWindowA(
		"EDIT", NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		0, 418, 754, 20, hWnd, (HMENU)101, hInst, NULL
	);
	mainCTX.lpfnInputProc = (WNDPROC)SetWindowLongPtr(mainCTX.hInput, GWLP_WNDPROC, (LONG_PTR)subInputProc);

	mainCTX.hSend = CreateWindowA(
		"BUTTON", "Send",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_DEFPUSHBUTTON,
		754, 418, 100, 20, hWnd, (HMENU)102, hInst, NULL
	);

	mainCTX.hList = CreateWindowA(
		"LISTBOX", NULL,
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
			if((HWND)wParam == hWnd || (HWND)wParam == mainCTX.hOutput) {
				SetCursor(hDefaultCursor);
				return TRUE;
			} else break;
		case WM_CONTEXTMENU:
			if((HWND)wParam == mainCTX.hList)
				OpenPlayerContextMenu(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_COMMAND:
			if((HWND)lParam == mainCTX.hSend) {
				if(LOWORD(wParam) == 102 && HIWORD(wParam) == BN_CLICKED) {
					ExecuteUserCommand();
					return TRUE;
				}
			} else if((HWND)lParam == mainCTX.hOutput) {
				if(HIWORD(wParam) == EN_SETFOCUS)
					HideCaret(mainCTX.hOutput);
			}
			return FALSE;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
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
			return CallWindowProcA(mainCTX.lpfnInputProc, hWnd, uMsg, wParam, lParam);
	}
}

void Backend_AddUser(cs_str name) {
	SendMessageA(mainCTX.hList, LB_ADDSTRING, (WPARAM)0, (LPARAM)name);
}

void Backend_RemoveUser(cs_str name) {
	cs_char tmp[64];
	for(cs_int32 i = 0; i < SendMessageA(mainCTX.hList, LB_GETCOUNT, 0, 0); i++) {
		LRESULT len = SendMessageA(mainCTX.hList, LB_GETTEXT, (WPARAM)i, (LPARAM)tmp);
		if(len && String_CaselessCompare2(name, tmp, len)) {
			SendMessageA(mainCTX.hList, LB_DELETESTRING, (WPARAM)i, (LPARAM)0);
			break;
		}
	}
}

void Backend_SetConsoleText(cs_str txt) {
	SendMessageA(mainCTX.hOutput, WM_SETTEXT, (WPARAM)0, (LPARAM)txt);
}

cs_size Backend_ScrollToEnd(void) {
	SendMessageA(mainCTX.hOutput, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
	SendMessageA(mainCTX.hOutput, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
	SendMessageA(mainCTX.hOutput, EM_SCROLLCARET, (WPARAM)0, (LPARAM)0);
	return 0;
}

cs_size Backend_GetScrollPosition(void) {
	return 0;
}

cs_size Backend_GetScrollEnd(void) {
	return 0;
}

cs_size Backend_GetInputText(cs_char *buff, cs_size len) {
	return SendMessageA(mainCTX.hInput, WM_GETTEXT, (WPARAM)len, (LPARAM)buff);
}

cs_bool Backend_ClearInputText(void) {
	return (cs_bool)SendMessageA(mainCTX.hInput, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
}

void Backend_PreLaunch(void) {}

void Backend_WindowLoop(void) {
	RegisterClassA(&mainCTX.wc);

	if((mainCTX.hWnd = CreateWindowA(
		mainCTX.wc.lpszClassName,
		CSGUI_WINDOWTITLE,
		WS_OVERLAPPEDWINDOW &
		~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT,
		870, 477, NULL, NULL, NULL, NULL
	)) == NULL) return;

	ShowWindow(mainCTX.hWnd, SW_SHOW);

	MSG msg;

	while(GetMessageA(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Backend_CloseWindow(void) {
	DestroyWindow(mainCTX.hWnd);
	UnregisterClassA(mainCTX.wc.lpszClassName, mainCTX.wc.hInstance);
}
