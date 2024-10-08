#define OEMRESOURCE
#include <windows.h>
#include <strsafe.h>
#include "resource.h"

#define VERSION "v1.1.3"

HINSTANCE   g_instance;
HCURSOR     g_hc_ibeam;
HCURSOR     g_hc_arrow;
UINT_PTR    g_timer = NULL;

NOTIFYICONDATA g_notifyIconData;
HMENU       g_hMenu;

int         g_layout = 0;

BOOL g_disable_arrow = FALSE;
BOOL g_disable_ibeam = FALSE;

BOOL g_autostart = TRUE;


//void LoadMenuStrings()
//{
//    TCHAR buffer[256];
//
//    LoadString(g_instance, ID_TRAY_DISABLE_ARROW, buffer, ARRAYSIZE(buffer));
//    AppendMenu(g_hMenu, MF_STRING | MF_UNCHECKED, ID_TRAY_DISABLE_ARROW, buffer);
//
//    LoadString(g_instance, ID_TRAY_DISABLE_IBEAM, buffer, ARRAYSIZE(buffer));
//    AppendMenu(g_hMenu, MF_STRING | MF_UNCHECKED, ID_TRAY_DISABLE_IBEAM, buffer);
//
//    LoadString(g_instance, ID_TRAY_EXIT, buffer, ARRAYSIZE(buffer));
//    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_EXIT, buffer);
//}


void SaveSettings()
{
    HKEY hKey;
    // Open or create a key in the registry
    if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\MovaFlag\\Settings", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        // Save the g_disable_ibeam value
        DWORD disableIbeam = g_disable_ibeam;
        RegSetValueEx(hKey, "DisableIbeam", 0, REG_DWORD, (const BYTE*)&disableIbeam, sizeof(disableIbeam));

        // Save the g_disable_arrow value
        DWORD disableArrow = g_disable_arrow;
        RegSetValueEx(hKey, "DisableArrow", 0, REG_DWORD, (const BYTE*)&disableArrow, sizeof(disableArrow));

        DWORD autostart = g_autostart;
        RegSetValueEx(hKey, "Autostart", 0, REG_DWORD, (const BYTE*)&autostart, sizeof(autostart));

        // Close the key
        RegCloseKey(hKey);
    }
}

void LoadSettings()
{
    HKEY hKey;
    DWORD disableIbeam = 0;
    DWORD disableArrow = 0;
    DWORD autostart = 1; // Default to autostart enabled
    DWORD size = sizeof(DWORD);

    // open the key in the registry
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\MovaFlag\\Settings", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        // read the g_disable_ibeam value
        RegQueryValueEx(hKey, "DisableIbeam", NULL, NULL, (LPBYTE)&disableIbeam, &size);
        g_disable_ibeam = (BOOL)disableIbeam;

        // read the g_disable_arrow value
        RegQueryValueEx(hKey, "DisableArrow", NULL, NULL, (LPBYTE)&disableArrow, &size);
        g_disable_arrow = (BOOL)disableArrow;

        RegQueryValueEx(hKey, "Autostart", NULL, NULL, (LPBYTE)&autostart, &size);
        g_autostart = (BOOL)autostart;

        // Close the key
        RegCloseKey(hKey);
    }
}


void AddToStartup()
{
    HKEY hKey;
    // Open or create a key in the registry
    if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        // Set the value to the path of your executable
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
        RegSetValueEx(hKey, "MovaFlag", 0, REG_SZ, (const BYTE*)szPath, (DWORD)((lstrlen(szPath) + 1) * sizeof(TCHAR)));

        // Close the key
        RegCloseKey(hKey);
    }
}

void RemoveFromStartup()
{
    HKEY hKey;
    // Open the registry key
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        // Delete the value
        RegDeleteValue(hKey, "MovaFlag");

        // Close the key
        RegCloseKey(hKey);
    }
}


void ChangeCursor(int cursor_id, DWORD cursor_type, bool disable_flag, HCURSOR default_cursor)
{
    if (!disable_flag)
    {
        HCURSOR new_cursor = LoadCursor(g_instance, MAKEINTRESOURCE(cursor_id));

        if (new_cursor)
            SetSystemCursor(new_cursor, cursor_type);
        else
            SetSystemCursor(CopyCursor(default_cursor), cursor_type);
    }
}

void CALLBACK UpdateTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    int layout = (int)GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)) & 0xFFFF;

    if (g_layout != layout)
    {
        ChangeCursor(layout + 100, OCR_NORMAL, g_disable_arrow, g_hc_arrow);
        ChangeCursor(layout, OCR_IBEAM, g_disable_ibeam, g_hc_ibeam);

        g_layout = layout;
    }
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case ID_TRAY_DISABLE_ARROW:
            g_disable_arrow = !g_disable_arrow;
            CheckMenuItem(g_hMenu, ID_TRAY_DISABLE_ARROW, g_disable_arrow ? MF_CHECKED : MF_UNCHECKED);

            SetSystemCursor(CopyCursor(g_hc_arrow), OCR_NORMAL);
            g_layout = 0;

            SaveSettings();
            break;

        case ID_TRAY_DISABLE_IBEAM:
            g_disable_ibeam = !g_disable_ibeam;
            CheckMenuItem(g_hMenu, ID_TRAY_DISABLE_IBEAM, g_disable_ibeam ? MF_CHECKED : MF_UNCHECKED);

            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
            g_layout = 0;

            SaveSettings();
            break;

        case ID_TRAY_AUTOSTART:
            g_autostart = !g_autostart;
            CheckMenuItem(g_hMenu, ID_TRAY_AUTOSTART, g_autostart ? MF_CHECKED : MF_UNCHECKED);

            if (g_autostart)
                AddToStartup();
            else
                RemoveFromStartup();

            SaveSettings();
            break;

        case ID_TRAY_EXIT:
            PostQuitMessage(0);
            break;
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


    g_instance = GetModuleHandle(NULL);


    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_instance;
    wc.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_APP_ICON)); // Set application icon
    wc.lpszClassName = "LangCursorClass";
    RegisterClass(&wc);

    HWND hWnd = CreateWindow("LangCursorClass", "MovaFlag", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_instance, NULL);
    if (!hWnd) return 1;

    g_hMenu = CreatePopupMenu();
    AppendMenu(g_hMenu, MF_STRING | MF_DISABLED | MF_GRAYED, 0, VERSION);
    AppendMenu(g_hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(g_hMenu, MF_STRING | MF_UNCHECKED, ID_TRAY_DISABLE_ARROW, "Disable Arrow Flag");
    AppendMenu(g_hMenu, MF_STRING | MF_UNCHECKED, ID_TRAY_DISABLE_IBEAM, "Disable IBeam Flag");
    AppendMenu(g_hMenu, MF_STRING | MF_UNCHECKED, ID_TRAY_AUTOSTART, "Run at Startup");
    AppendMenu(g_hMenu, MF_STRING, ID_TRAY_EXIT, "Quit");
    //LoadMenuStrings(); // Load localized menu strings

    g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    g_notifyIconData.hWnd = hWnd;
    g_notifyIconData.uID = 1;
    g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_notifyIconData.uCallbackMessage = WM_USER + 1;
    g_notifyIconData.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_TRAY_ICON)); // Set tray icon
    StringCchCopy(g_notifyIconData.szTip, ARRAYSIZE(g_notifyIconData.szTip), "MovaFlag");
    Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);


    LoadSettings();
    if (g_disable_ibeam)
        CheckMenuItem(g_hMenu, ID_TRAY_DISABLE_IBEAM, MF_CHECKED);
    else
        CheckMenuItem(g_hMenu, ID_TRAY_DISABLE_IBEAM, MF_UNCHECKED);

    if (g_disable_arrow)
        CheckMenuItem(g_hMenu, ID_TRAY_DISABLE_ARROW, MF_CHECKED);
    else
        CheckMenuItem(g_hMenu, ID_TRAY_DISABLE_ARROW, MF_UNCHECKED);

    if (g_autostart){
        CheckMenuItem(g_hMenu, ID_TRAY_AUTOSTART, MF_CHECKED);
        AddToStartup();
    }     
    else {
        CheckMenuItem(g_hMenu, ID_TRAY_AUTOSTART, MF_UNCHECKED);
        RemoveFromStartup();
    }
        

    g_hc_ibeam = CopyCursor(LoadCursor(NULL, IDC_IBEAM));
    if (!g_hc_ibeam)
        return 1;

    g_hc_arrow = CopyCursor(LoadCursor(NULL, IDC_ARROW));
    if (!g_hc_arrow)
        return 1;


    g_timer = SetTimer(NULL, g_timer, 200, UpdateTimer);
    if (!g_timer)
        return 1;

    
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
    SetSystemCursor(CopyCursor(g_hc_arrow), OCR_NORMAL);

    DestroyCursor(g_hc_ibeam);
    DestroyCursor(g_hc_arrow);

    SaveSettings();

    return 0;
}

EXTERN_C void WINAPI WinMainCRTStartup()
{
    ExitProcess(Main());
}
