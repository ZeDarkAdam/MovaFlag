#define OEMRESOURCE
#include <windows.h>

HINSTANCE   g_instance;
HCURSOR     g_hc_ibeam;
HCURSOR     g_hc_arrow;
UINT_PTR    g_timer = NULL;
DWORD       g_layout = 0;

void CALLBACK UpdateTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    int layout = (int)GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)) & 0xFFFF; // Отримання поточної мови введення
    int caps = GetKeyState(VK_CAPITAL) & 0xFFFF; // Перевірка стану Caps Lock
   
    //HCURSOR hc_new_arrow = NULL;
    //switch (layout)
    //{
    //case 1033: // English (USA)
    //    hc_new_arrow = LoadCursor(g_instance, MAKEINTRESOURCE(layout + 100));
    //    break;
    //case 1049: // Russian
    //    hc_new_arrow = LoadCursor(g_instance, MAKEINTRESOURCE(layout + 100));
    //    break;
    //case 1058: // Ukrainian
    //    hc_new_arrow = LoadCursor(g_instance, MAKEINTRESOURCE(layout + 100));
    //    break;
    //default:
    //    hc_new_arrow = CopyCursor(g_hc_arrow);
    //    break;
    //}

    HCURSOR hc_new_arrow = LoadCursor(g_instance, MAKEINTRESOURCE(layout + 100));

    if (caps) {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE((layout * 10) + 2));
       
        if (hc_new)
            SetSystemCursor(hc_new, OCR_IBEAM);
        else
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);

        if (hc_new_arrow)
            SetSystemCursor(hc_new_arrow, OCR_NORMAL);
        else
            SetSystemCursor(CopyCursor(g_hc_arrow), OCR_NORMAL);
    }
    
    if (g_layout != layout && !caps)
    {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE(layout));

        if (hc_new)
            SetSystemCursor(hc_new, OCR_IBEAM);
        else
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);

        if (hc_new_arrow)
            SetSystemCursor(hc_new_arrow, OCR_NORMAL);
        else
            SetSystemCursor(CopyCursor(g_hc_arrow), OCR_NORMAL);


    }
}

int Main()
{
    HANDLE mutex = CreateMutex(NULL, FALSE, "LangCursor");
    if (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED) 
        return 1;

    g_hc_ibeam = CopyCursor(LoadCursor(NULL, IDC_IBEAM));
    if (!g_hc_ibeam) return 1;

    g_hc_arrow = CopyCursor(LoadCursor(NULL, IDC_ARROW));
    if (!g_hc_arrow) return 1;

    // Gets a handle to the current application instance
    g_instance = GetModuleHandle(NULL);

    g_timer = SetTimer(NULL, g_timer, 200, UpdateTimer);

    if (!g_timer) 
        return 1;

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyCursor(g_hc_ibeam);
    DestroyCursor(g_hc_arrow);

    return 0;
}

EXTERN_C void WINAPI WinMainCRTStartup()
{
    ExitProcess(Main());
}
