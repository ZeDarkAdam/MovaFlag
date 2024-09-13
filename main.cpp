#define OEMRESOURCE
#include <windows.h>
#include <strsafe.h>
#include "resource.h"

HINSTANCE   g_instance;
HCURSOR     g_hc_ibeam;
HCURSOR     g_hc_arrow;
UINT_PTR    g_timer = NULL;

NOTIFYICONDATA g_notifyIconData;
HMENU       g_hMenu;

int         g_layout = 0;

void CALLBACK UpdateTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    int layout = (int)GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)) & 0xFFFF;
    //int caps = GetKeyState(VK_CAPITAL) & 0xFFFF;

    /*if (caps) {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE((layout * 10) + 2));

        if (hc_new)
            SetSystemCursor(hc_new, OCR_IBEAM);
        else
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
    }
    else {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE(layout));

        if (hc_new)
            SetSystemCursor(hc_new, OCR_IBEAM);     
        else
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
    }*/

    if (g_layout != layout)
    {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE(layout));

        if (hc_new)
            SetSystemCursor(hc_new, OCR_IBEAM);
        else
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);



        HCURSOR hc_new_arrow = LoadCursor(g_instance, MAKEINTRESOURCE(layout + 100));

        if (hc_new_arrow)
            SetSystemCursor(hc_new_arrow, OCR_NORMAL);
        else
            SetSystemCursor(CopyCursor(g_hc_arrow), OCR_NORMAL);

        g_layout = layout;
    }

    //g_layout = layout;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
        PostQuitMessage(0);
        break;
    case WM_USER + 1:
        if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hWnd);
            TrackPopupMenu(g_hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

int Main()
{
    HANDLE mutex = CreateMutex(NULL, FALSE, "MovaFlag");
    if (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED)
        return 1;

    g_hc_ibeam = CopyCursor(LoadCursor(NULL, IDC_IBEAM));
    if (!g_hc_ibeam)
        return 1;

    g_hc_arrow = CopyCursor(LoadCursor(NULL, IDC_ARROW));
    if (!g_hc_arrow)
        return 1;

    g_instance = GetModuleHandle(NULL);
    g_timer = SetTimer(NULL, g_timer, 200, UpdateTimer);
    if (!g_timer)
        return 1;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_instance;
    wc.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_APP_ICON)); // Set application icon
    wc.lpszClassName = "LangCursorClass";
    RegisterClass(&wc);

    HWND hWnd = CreateWindow("LangCursorClass", "MovaFlag", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_instance, NULL);
    if (!hWnd) return 1;

    g_hMenu = CreatePopupMenu();
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");

    g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    g_notifyIconData.hWnd = hWnd;
    g_notifyIconData.uID = 1;
    g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_notifyIconData.uCallbackMessage = WM_USER + 1;
    g_notifyIconData.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_TRAY_ICON)); // Set tray icon
    StringCchCopy(g_notifyIconData.szTip, ARRAYSIZE(g_notifyIconData.szTip), "MovaFlag");
    Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    SetSystemCursor(g_hc_ibeam, OCR_IBEAM);
    SetSystemCursor(g_hc_arrow, OCR_NORMAL);

    DestroyCursor(g_hc_ibeam);
    DestroyCursor(g_hc_arrow);
    return 0;
}

EXTERN_C void WINAPI WinMainCRTStartup()
{
    ExitProcess(Main());
}
