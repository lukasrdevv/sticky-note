#ifndef UNICODE
#define UNICODE
#endif

#define _WIN32_WINNT 0x0A00 // Windows 10
#define WINVER 0x0A00

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#include "resource.h"

// Dimensions & Constants
#define NOTE_WIDTH 250
#define NOTE_HEIGHT 250
#define HEADER_HEIGHT 25
#define BG_COLOR RGB(255, 255, 150)
#define ID_TRAY_APP_ICON 1001
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM 3000
#define ID_TRAY_TOGGLE_CONTEXT_MENU_ITEM 3001
#define ID_EDIT_CHILD 1002
#define WM_TRAYICON (WM_USER + 1)

// Global variables
HBRUSH hBrushYellow;
HFONT hFont;
NOTIFYICONDATA nid;
bool isVisible = true;
HWND hEdit; // Global handle to edit control
bool isLoading = false; // Flag to prevent save loop during load

const wchar_t DATA_FILE[] = L"sticky_data.dat";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Helper: Save content to file (Raw UTF-16LE)
void SaveNote() {
    if (isLoading) return;
    if (!hEdit) return;

    int len = GetWindowTextLength(hEdit);
    // Even if 0, we save empty file
    
    HANDLE hFile = CreateFile(DATA_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    if (len > 0) {
        wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) * sizeof(wchar_t));
        if (buffer) {
            GetWindowText(hEdit, buffer, len + 1);
            DWORD bytesWritten;
            WriteFile(hFile, buffer, len * sizeof(wchar_t), &bytesWritten, NULL);
            HeapFree(GetProcessHeap(), 0, buffer);
        }
    }
    
    CloseHandle(hFile);
}

// Helper: Load content from file
void LoadNote() {
    HANDLE hFile = CreateFile(DATA_FILE, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > 0) {
        wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + sizeof(wchar_t));
        if (buffer) {
            DWORD bytesRead;
            if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                isLoading = true;
                SetWindowText(hEdit, buffer);
                isLoading = false;
            }
            HeapFree(GetProcessHeap(), 0, buffer);
        }
    }
    CloseHandle(hFile);
}

void InitNotifyIcon(HWND hwnd) {
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_APP_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    // Load Custom Icon
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APP_ICON)); 
    lstrcpy(nid.szTip, L"Sticky Note");
    Shell_NotifyIcon(NIM_ADD, &nid);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const wchar_t CLASS_NAME[] = L"StickyNoteHost";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); 
    // Load Custom Icon for Class as well (visible in Alt-Tab if toolwindow wasn't set, or task manager)
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    RegisterClass(&wc);

    hBrushYellow = CreateSolidBrush(BG_COLOR);

    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.left + 20;
    int y = workArea.bottom - NOTE_HEIGHT - 20;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW, 
        CLASS_NAME,
        L"Sticky Note",
        WS_POPUP | WS_VISIBLE, 
        x, y, NOTE_WIDTH, NOTE_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;

    InitNotifyIcon(hwnd);

    // Initial Load
    LoadNote();

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    DeleteObject(hBrushYellow);
    if (hFont) DeleteObject(hFont);

    return 0;
}

void ShowContextMenu(HWND hwnd, POINT pt) {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        if (isVisible)
            InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_TOGGLE_CONTEXT_MENU_ITEM, L"Hide Note");
        else
            InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_TOGGLE_CONTEXT_MENU_ITEM, L"Show Note");
        
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, L"Exit");
        
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        {
            hEdit = CreateWindowEx(
                0, L"EDIT",
                L"", // Empty initially, loaded later
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL, 
                0, HEADER_HEIGHT, NOTE_WIDTH, NOTE_HEIGHT - HEADER_HEIGHT,
                hwnd,
                (HMENU)ID_EDIT_CHILD,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );

            HDC hDC = GetDC(hwnd);
            int nHeight = -MulDiv(11, GetDeviceCaps(hDC, LOGPIXELSY), 72);
            ReleaseDC(hwnd, hDC);

            hFont = CreateFont(nHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(10, 10)); 
            
            // Limit text limit just in case? Default is 32k usually enough.
            SendMessage(hEdit, EM_SETLIMITTEXT, 0, 0); 
        }
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rcHeader;
            GetClientRect(hwnd, &rcHeader);
            rcHeader.bottom = HEADER_HEIGHT;
            FillRect(hdc, &rcHeader, hBrushYellow);
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            MoveWindow(hEdit, 0, HEADER_HEIGHT, width, height - HEADER_HEIGHT, TRUE);
        }
        return 0;

    case WM_NCHITTEST:
        {
            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINTS pt = MAKEPOINTS(lParam);
                RECT rc;
                GetWindowRect(hwnd, &rc);
                if (pt.y - rc.top < HEADER_HEIGHT) {
                    return HTCAPTION; 
                }
            }
            return hit;
        }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkColor(hdc, BG_COLOR);
            return (LRESULT)hBrushYellow;
        }

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT curPoint;
            GetCursorPos(&curPoint);
            ShowContextMenu(hwnd, curPoint);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT_CONTEXT_MENU_ITEM:
            SaveNote(); // Ensure saved on exit
            PostQuitMessage(0);
            break;
        case ID_TRAY_TOGGLE_CONTEXT_MENU_ITEM:
            if (isVisible) {
                ShowWindow(hwnd, SW_HIDE);
                isVisible = false;
            } else {
                ShowWindow(hwnd, SW_SHOW);
                isVisible = true;
            }
            break;
        case ID_EDIT_CHILD:
            // Check for EN_CHANGE notification
            if (HIWORD(wParam) == EN_CHANGE) {
                SaveNote();
            }
            break;
        }
        return 0;

    case WM_DESTROY:
        SaveNote(); // Final save
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}