/*
 * NV Container Fuck Off v1.2.0 - by HXPRO
 * Minimal native Windows utility. MIT License.
 * Only manages the exact service NVDisplay.ContainerLocalSystem.
 * No PowerShell, CMD execution, network, data collection or persistence.
 */
#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <winsvc.h>
#include <strsafe.h>

#define APP_CLASS L"HXPRO_NVContainerFuckOff_v120"
#define SERVICE_NAME L"NVDisplay.ContainerLocalSystem"
#define ID_OFF 1001
#define ID_ON 1002
#define ID_REFRESH 1003

static HWND g_main, g_status, g_details, g_off, g_on, g_refresh;
static HFONT g_titleFont, g_normalFont, g_statusFont, g_smallFont;
static HBRUSH g_background;

static void set_font(HWND ctl, HFONT font) {
    SendMessageW(ctl, WM_SETFONT, (WPARAM)font, TRUE);
}

static HWND make_label(int x, int y, int width, int height, LPCWSTR content, HFONT font) {
    HWND h = CreateWindowExW(0, L"STATIC", content, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, width, height, g_main, NULL, GetModuleHandleW(NULL), NULL);
    if (h) set_font(h, font);
    return h;
}

static HWND make_button(int id, LPCWSTR caption, int x, int y, int width, int height) {
    HWND h = CreateWindowExW(0, L"BUTTON", caption, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        x, y, width, height, g_main, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
    if (h) set_font(h, g_normalFont);
    return h;
}

static void show_error(DWORD error, LPCWSTR stage) {
    WCHAR message[700], reason[480];
    DWORD count = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, 0, reason, (DWORD)(sizeof(reason)/sizeof(reason[0])), NULL);
    if (!count) StringCchCopyW(reason, sizeof(reason)/sizeof(reason[0]), L"Unknown Windows error.");
    StringCchPrintfW(message, sizeof(message)/sizeof(message[0]),
        L"%s\n\nWindows error %lu: %s", stage, error, reason);
    MessageBoxW(g_main, message, L"NV Container Fuck Off - Operation failed", MB_OK | MB_ICONERROR);
}

static BOOL query_status(SC_HANDLE service, DWORD *state) {
    SERVICE_STATUS_PROCESS status;
    DWORD bytesNeeded = 0;
    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&status,
            sizeof(status), &bytesNeeded)) return FALSE;
    *state = status.dwCurrentState;
    return TRUE;
}

static BOOL wait_for_state(SC_HANDLE service, DWORD goal) {
    ULONGLONG deadline = GetTickCount64() + 12000;
    DWORD current = 0;
    do {
        if (!query_status(service, &current)) return FALSE;
        if (current == goal) return TRUE;
        if (current != SERVICE_START_PENDING && current != SERVICE_STOP_PENDING &&
            current != SERVICE_CONTINUE_PENDING) {
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
            return FALSE;
        }
        Sleep(150);
    } while (GetTickCount64() < deadline);
    SetLastError(ERROR_TIMEOUT);
    return FALSE;
}

static BOOL change_start_type(SC_HANDLE service, DWORD startType) {
    return ChangeServiceConfigW(service, SERVICE_NO_CHANGE, startType, SERVICE_NO_CHANGE,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

/* Returns 1 for success, 0 for failure; first failing Win32 error is preserved. */
static BOOL apply_service_state(BOOL turnOn) {
    SC_HANDLE manager = NULL, service = NULL;
    DWORD state = 0, err = ERROR_SUCCESS;
    SERVICE_STATUS oldStatus;
    BOOL ok = FALSE;

    manager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!manager) return FALSE;
    service = OpenServiceW(manager, SERVICE_NAME,
        SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG | SERVICE_START | SERVICE_STOP);
    if (!service) {
        err = GetLastError();
        goto finish;
    }
    if (!query_status(service, &state)) {
        err = GetLastError();
        goto finish;
    }

    if (turnOn) {
        if (!change_start_type(service, SERVICE_AUTO_START)) {
            err = GetLastError();
            goto finish;
        }
        if (state == SERVICE_STOP_PENDING) {
            if (!wait_for_state(service, SERVICE_STOPPED)) {
                err = GetLastError();
                goto finish;
            }
            state = SERVICE_STOPPED;
        }
        if (state == SERVICE_STOPPED) {
            if (!StartServiceW(service, 0, NULL)) {
                err = GetLastError();
                goto finish;
            }
        }
        if (state != SERVICE_RUNNING && !wait_for_state(service, SERVICE_RUNNING)) {
            err = GetLastError();
            goto finish;
        }
    } else {
        /* Stop first. If stopping fails, do not silently disable a running service. */
        if (state == SERVICE_RUNNING || state == SERVICE_PAUSED) {
            if (!ControlService(service, SERVICE_CONTROL_STOP, &oldStatus)) {
                err = GetLastError();
                goto finish;
            }
            state = SERVICE_STOP_PENDING;
        }
        if (state != SERVICE_STOPPED && !wait_for_state(service, SERVICE_STOPPED)) {
            err = GetLastError();
            goto finish;
        }
        if (!change_start_type(service, SERVICE_DISABLED)) {
            err = GetLastError();
            goto finish;
        }
    }
    ok = TRUE;

finish:
    if (service) CloseServiceHandle(service);
    if (manager) CloseServiceHandle(manager);
    if (!ok) SetLastError(err);
    return ok;
}

static void refresh_service(void) {
    SC_HANDLE manager = NULL, service = NULL;
    QUERY_SERVICE_CONFIGW *config = NULL;
    DWORD state = 0, required = 0, error = 0;
    BOOL isOff, isOn;

    manager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!manager) {
        error = GetLastError();
        goto failed;
    }
    service = OpenServiceW(manager, SERVICE_NAME, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (!service) {
        error = GetLastError();
        goto failed;
    }
    if (!query_status(service, &state)) {
        error = GetLastError();
        goto failed;
    }
    QueryServiceConfigW(service, NULL, 0, &required);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || !required || required > 65536) {
        error = GetLastError();
        goto failed;
    }
    config = (QUERY_SERVICE_CONFIGW *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, required);
    if (!config) {
        error = ERROR_OUTOFMEMORY;
        goto failed;
    }
    if (!QueryServiceConfigW(service, config, required, &required)) {
        error = GetLastError();
        goto failed;
    }

    isOff = state == SERVICE_STOPPED && config->dwStartType == SERVICE_DISABLED;
    isOn = state == SERVICE_RUNNING && config->dwStartType == SERVICE_AUTO_START;
    SetWindowTextW(g_status, isOff ? L"OFF  -  Stopped / Disabled" :
        isOn ? L"ON  -  Running / Automatic" : L"CUSTOM  -  See service configuration");
    SetWindowTextW(g_details, SERVICE_NAME);
    EnableWindow(g_off, !isOff);
    EnableWindow(g_on, !isOn);
    goto cleanup;

failed:
    if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
        SetWindowTextW(g_status, L"SERVICE NOT FOUND");
        SetWindowTextW(g_details, L"Install the NVIDIA display driver before using this tool.");
    } else {
        WCHAR msg[200];
        StringCchPrintfW(msg, sizeof(msg)/sizeof(msg[0]), L"Unable to read service (error %lu).", error);
        SetWindowTextW(g_status, L"SERVICE STATUS UNAVAILABLE");
        SetWindowTextW(g_details, msg);
    }
    EnableWindow(g_off, FALSE);
    EnableWindow(g_on, FALSE);

cleanup:
    if (config) HeapFree(GetProcessHeap(), 0, config);
    if (service) CloseServiceHandle(service);
    if (manager) CloseServiceHandle(manager);
}

static void click_toggle(BOOL turnOn) {
    DWORD err;
    if (!turnOn && MessageBoxW(g_main,
        L"Turn OFF NVIDIA Display Container LS?\n\n"
        L"NVIDIA Control Panel, the tray icon and some display settings may become unavailable.\n\n"
        L"TURN ON restores the service when needed.",
        L"Confirm TURN OFF", MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING) != IDYES) return;

    EnableWindow(g_off, FALSE);
    EnableWindow(g_on, FALSE);
    EnableWindow(g_refresh, FALSE);
    SetWindowTextW(g_status, L"Applying change...");
    UpdateWindow(g_main);
    if (!apply_service_state(turnOn)) {
        err = GetLastError();
        show_error(err, turnOn ? L"Could not enable/start the service." : L"Could not stop/disable the service.");
    }
    EnableWindow(g_refresh, TRUE);
    refresh_service();
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE: {
            LOGFONTW lf;
            g_main = hwnd;
            ZeroMemory(&lf, sizeof(lf));
            StringCchCopyW(lf.lfFaceName, LF_FACESIZE, L"Segoe UI");
            lf.lfHeight = -21; lf.lfWeight = FW_NORMAL;
            g_normalFont = CreateFontIndirectW(&lf);
            lf.lfHeight = -29; lf.lfWeight = FW_BOLD;
            g_titleFont = CreateFontIndirectW(&lf);
            lf.lfHeight = -25; lf.lfWeight = FW_BOLD;
            g_statusFont = CreateFontIndirectW(&lf);
            lf.lfHeight = -16; lf.lfWeight = FW_NORMAL;
            g_smallFont = CreateFontIndirectW(&lf);
            g_background = CreateSolidBrush(RGB(245,247,251));

            make_label(24, 18, 520, 38, L"NV CONTAINER FUCK OFF", g_titleFont);
            make_label(24, 61, 500, 25, L"NVIDIA Display Container LS  -  service switch", g_normalFont);
            make_label(25, 110, 500, 23, L"CURRENT STATUS", g_smallFont);
            g_status = make_label(25, 137, 520, 34, L"Checking...", g_statusFont);
            g_details = make_label(25, 179, 520, 24, SERVICE_NAME, g_smallFont);
            g_off = make_button(ID_OFF, L"TURN OFF", 24, 220, 247, 52);
            g_on = make_button(ID_ON, L"TURN ON", 289, 220, 247, 52);
            g_refresh = make_button(ID_REFRESH, L"REFRESH STATUS", 24, 291, 174, 37);
            make_label(212, 298, 324, 27, L"by HXPRO  |  v1.2.0  |  Unofficial", g_smallFont);
            make_label(24, 350, 520, 24, L"Only the NVIDIA Display Container LS service is modified.", g_smallFont);
            return 0;
        }
        case WM_SHOWWINDOW:
            if (wparam) refresh_service();
            return 0;
        case WM_COMMAND:
            if (HIWORD(wparam) == BN_CLICKED) {
                switch (LOWORD(wparam)) {
                    case ID_OFF: click_toggle(FALSE); return 0;
                    case ID_ON: click_toggle(TRUE); return 0;
                    case ID_REFRESH: refresh_service(); return 0;
                }
            }
            return 0;
        case WM_CTLCOLORSTATIC:
            SetBkColor((HDC)wparam, RGB(245,247,251));
            SetTextColor((HDC)wparam, RGB(35,45,60));
            return (LRESULT)g_background;
        case WM_DESTROY:
            if (g_titleFont) DeleteObject(g_titleFont);
            if (g_normalFont) DeleteObject(g_normalFont);
            if (g_statusFont) DeleteObject(g_statusFont);
            if (g_smallFont) DeleteObject(g_smallFont);
            if (g_background) DeleteObject(g_background);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR commandLine, int showCommand) {
    WNDCLASSEXW klass;
    HWND window;
    MSG msg;
    RECT work;
    int width = 565, height = 430;
    (void)previous; (void)commandLine; (void)showCommand;
    ZeroMemory(&klass, sizeof(klass));
    klass.cbSize = sizeof(klass);
    klass.lpfnWndProc = window_proc;
    klass.hInstance = instance;
    klass.lpszClassName = APP_CLASS;
    klass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    klass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    klass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    if (!RegisterClassExW(&klass)) return 1;

    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    window = CreateWindowExW(0, APP_CLASS, L"NV Container Fuck Off - by HXPRO",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        work.left + ((work.right-work.left)-width)/2,
        work.top + ((work.bottom-work.top)-height)/2,
        width, height, NULL, NULL, instance, NULL);
    if (!window) return 2;
    g_main = window;
    ShowWindow(window, SW_SHOWNORMAL);
    UpdateWindow(window);
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
