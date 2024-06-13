#define OEMRESOURCE
#include <windows.h>

HINSTANCE   g_instance;
HCURSOR     g_hc_ibeam;
UINT_PTR    g_timer = NULL;
DWORD       g_layout = 0;

void CALLBACK UpdateTimer(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    int layout = (int)GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)) & 0xFFFF; // Отримання поточної мови введення
    int caps = GetKeyState(VK_CAPITAL) & 0xFFFF; // Перевірка стану Caps Lock
   
    if (caps) {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE((layout * 10) + 2));
       
        if (hc_new)
        {
            SetSystemCursor(hc_new, OCR_IBEAM);
        }
        else
        {
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
        }
    }
    
    if (g_layout != layout && !caps)
    {
        HCURSOR hc_new = LoadCursor(g_instance, MAKEINTRESOURCE(layout));

        if (hc_new)
        {
            SetSystemCursor(hc_new, OCR_IBEAM);
        }
        else
        {
            SetSystemCursor(CopyCursor(g_hc_ibeam), OCR_IBEAM);
        }
    }
}

int Main()
{
    HANDLE mutex = CreateMutex(NULL, FALSE, "LangCursor");
    if (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED) 
        return 1;

    // Виклик функції LoadCursor завантажує системний курсор з ідентифікатором IDC_IBEAM (текстовий курсор) 
    // і копіює його, використовуючи CopyCursor. Отриманий дескриптор копіюється в глобальну змінну g_hc_ibeam
    g_hc_ibeam = CopyCursor(LoadCursor(NULL, IDC_IBEAM));
    if (!g_hc_ibeam) return 1;

    // Отримання дескриптора поточного екземпляру програми
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
    return 0;
}

EXTERN_C void WINAPI WinMainCRTStartup()
{
    ExitProcess(Main());
}
