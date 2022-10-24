// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// we need commctrl v6 for LoadIconMetric()
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#include "resource.h"
#include "framework.h"
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <strsafe.h>
#include <Mmdeviceapi.h>
#include "TMMNotificationClient.h"
#include "CMMSession.h"
#include "CMMDevice.h"
#include "CMMDeviceController.h"
#include "Dbt.h"
#include <usbiodef.h>
#include <list>
#include "KBCommService.h"

using namespace ATL;

// Custom windows messages
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;
UINT const WMAPP_HIDEFLYOUT     = WM_APP + 2;

UINT const WMAPP_REFRESH_SESSION = WM_APP + 3;
UINT const WMAPP_REFRESH_DEVICE_SESSIONS = WM_APP + 4;
UINT const WMAPP_REFRESH_DEVICE_PROPERTIES = WM_APP + 5;
UINT const WMAPP_REFRESH_AUDIOCONTROLLER = WM_APP + 6;

// Global variables
HINSTANCE g_hInst = NULL;
HWND g_hwndMainWin = NULL;
CMMDeviceController* g_pMMDeviceController = NULL;
KBCommService* g_pKBCommService = NULL;
//HDEVNOTIFY g_hDeviceNotify = NULL;
std::list<WMAPPMessageHandler> g_WMAPPMessageHanders;

typedef struct RefreshSessionData
{
    MMSessionID sessionID;
    std::wstring reason;
} RefreshSessionData;

typedef struct RefreshDeviceData
{
    MMDeviceID deviceID;
    std::wstring reason;
} RefreshDeviceData;

typedef struct RefreshControllerData
{
    std::wstring reason;
} RefreshControllerData;

void PostMainThreadRefreshSession(MMSessionID sessionID, LPCWSTR pszReason) throw ()
{
    ATLTRACE(L"PostMainThreadRefreshSession(MSSessionID=%s, reason=%s)\n", sessionID.ToString(), pszReason);

    // pRefreshData object will be passed to the main thread.
    // When the main thread is done, the main thread handler will 
    // delete pRefreshData
    RefreshSessionData* pRefreshData = new RefreshSessionData();
    pRefreshData->sessionID = sessionID;
    pRefreshData->reason = pszReason;
    PostMessage(g_hwndMainWin, WMAPP_REFRESH_SESSION, (WPARAM)pRefreshData, 0);
}

void PostMainThreadRefreshDeviceProperties(MMDeviceID deviceID, LPCWSTR pszReason) throw ()
{
    ATLTRACE(L"PostMainThreadRefreshDeviceProperties(MMDeviceID=%s, reason=%s)\n", deviceID.ToString(), pszReason);

    // pRefreshData object will be passed to the main thread.
    // When the main thread is done, the main thread handler will 
    // delete pRefreshData
    RefreshDeviceData* pRefreshData = new RefreshDeviceData();
    pRefreshData->deviceID = deviceID;
    pRefreshData->reason = pszReason;
    PostMessage(g_hwndMainWin, WMAPP_REFRESH_DEVICE_PROPERTIES, (WPARAM)pRefreshData, 0);
}

void PostMainThreadRefreshDeviceSessions(MMDeviceID deviceID, LPCWSTR pszReason) throw ()
{
    ATLTRACE(L"PostMainThreadRefreshDeviceSessions(MMDeviceID=%s, reason=%s)\n", deviceID.ToString(), pszReason);

    // pRefreshData object will be passed to the main thread.
    // When the main thread is done, the main thread handler will 
    // delete pRefreshData
    RefreshDeviceData* pRefreshData = new RefreshDeviceData();
    pRefreshData->deviceID = deviceID;
    pRefreshData->reason = pszReason;
    PostMessage(g_hwndMainWin, WMAPP_REFRESH_DEVICE_SESSIONS, (WPARAM)pRefreshData, 0);
}

void PostMainThreadRefreshAudioController(LPCWSTR pszReason) throw ()
{
    ATLTRACE(L"PostMainThreadRefreshAudioController(reason=%s)\n", pszReason);

    // pRefreshData object will be passed to the main thread.
    // When the main thread is done, the main thread handler will 
    // delete pRefreshData
    RefreshControllerData* pRefreshData = new RefreshControllerData();
    pRefreshData->reason = pszReason;
    PostMessage(g_hwndMainWin, WMAPP_REFRESH_AUDIOCONTROLLER, (WPARAM)pRefreshData, 0);
}

HWND GetAppHWND()
{
    return g_hwndMainWin;
}

void AddWMAPPMessageHander(const WMAPPMessageHandler& handler)
{
    g_WMAPPMessageHanders.push_back( handler );
}

void RemoveWMAPPMessageHanderByID(DWORD_PTR handlerID)
{
    g_WMAPPMessageHanders.remove_if([handlerID](const WMAPPMessageHandler& handler) -> bool {
            return handler.id == handlerID;
        });
}


UINT_PTR const HIDEFLYOUT_TIMER_ID = 1;

wchar_t const szWindowClass[] = L"NotificationIconTest";
wchar_t const szFlyoutWindowClass[] = L"NotificationFlyout";

// Use a guid to uniquely identify our icon
// Mone: updated GUID every time the app restarts. Otherwise
// the icon may disappear if it's relaunched soon after a previous crash.
GUID PrinterIconGUID;

// Forward declarations of functions included in this code module:
void                RegisterWindowClass(PCWSTR pszClassName, PCWSTR pszMenuName, WNDPROC lpfnWndProc);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM                RegisterFlyoutClass(HINSTANCE hInstance);
LRESULT CALLBACK    FlyoutWndProc(HWND, UINT, WPARAM, LPARAM);
HWND                ShowFlyout(HWND hwnd);
void                HideFlyout(HWND hwndMainWindow, HWND hwndFlyout);
void                PositionFlyout(HWND hwnd, REFGUID guidIcon);
void                ShowContextMenu(HWND hwnd, POINT pt);
BOOL                AddNotificationIcon(HWND hwnd);
BOOL                DeleteNotificationIcon();
BOOL                ShowLowInkBalloon();
BOOL                ShowNoInkBalloon();
BOOL                ShowPrintJobBalloon();
BOOL                RestoreTooltip();

class CATLProject1Module : public ATL::CAtlExeModuleT< CATLProject1Module >
{
public:
    //DECLARE_LIBID(LIBID_ATLProject1Lib)
    //DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ATLPROJECT1, "{fedd9863-fabc-4c3b-8b21-d68c0f1bdb7d}")

    HRESULT Run(int nShowCmd = SW_HIDE) throw()
    {
        //Mone: set application DPI awareness for all the HWNDs going to be created.
        DPI_AWARENESS_CONTEXT context = nullptr;
        context = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
        SetThreadDpiAwarenessContext(context);

        HRESULT hr = S_OK;

        // Mone: updated GUID every time the app restarts. Otherwise
        hr = CoCreateGuid( &PrinterIconGUID );
        if (FAILED(hr)) {
            ATLTRACE(L"CoCreateGuid failed\n");
            return hr;
        }

//        g_hInst = hInstance;
        RegisterWindowClass(szWindowClass, MAKEINTRESOURCE(IDC_NOTIFICATIONICON), WndProc);
        RegisterWindowClass(szFlyoutWindowClass, NULL, FlyoutWndProc);

        // Create the main window. This could be a hidden window if you don't need
        // any UI other than the notification icon.
        WCHAR szTitle[100];
        LoadString(g_hInst, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));
        HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, 250, 200, NULL, NULL, g_hInst, NULL);
        if (hwnd)
        {
            //Mone: store hwnd created. Replace options dlg with this main hwnd
            g_hwndMainWin = hwnd;

            int hid_ret = hid_init();
            if (HID_FAILED(hid_ret))
            {
                return E_FAIL;
            }

            try
            {
                g_pMMDeviceController = CMMDeviceController::CreateObject();
#ifdef _DEBUG
                // print out all devices and sessions
                ATLTRACE(TEXT("CMMDeviceController::CreateObject\n"));
                g_pMMDeviceController->dump();
#endif

                g_pKBCommService = new KBCommService(g_pMMDeviceController);
                if (g_pKBCommService == NULL)
                {
                    CHK_HR(E_OUTOFMEMORY);
                }
                g_pKBCommService->Start();
            }
            catch (HRESULT hr_)
            {
                return hr_;
            }


            // Mone: hide main window on create
            //ShowWindow(hwnd, nCmdShow);

            // Main message loop:
            this->RunMessageLoop();
        } // if (hwnd)

        if (g_pKBCommService)
        {
            g_pKBCommService->Stop();
            delete g_pKBCommService;
            g_pKBCommService = NULL;
        }

        if (g_pMMDeviceController)
        {
            delete g_pMMDeviceController;
            g_pMMDeviceController = NULL;
        }

        hid_exit();


        return E_FAIL;

    }
};

CATLProject1Module _AtlModule;


int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
){
    g_hInst = hInstance;
    return _AtlModule.WinMain(nShowCmd);

}

void RegisterWindowClass(PCWSTR pszClassName, PCWSTR pszMenuName, WNDPROC lpfnWndProc)
{
    WNDCLASSEX wcex = {sizeof(wcex)};
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = lpfnWndProc;
    wcex.hInstance      = g_hInst;
    wcex.hIcon          = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NOTIFICATIONICON));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = pszMenuName;
    wcex.lpszClassName  = pszClassName;
    RegisterClassEx(&wcex);
}

BOOL AddNotificationIcon(HWND hwnd)
{
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.hWnd = hwnd;
    // add the icon, setting the icon, tooltip, and callback message.
    // the icon will be identified with the GUID
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = PrinterIconGUID;
    nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_SMALL, &nid.hIcon);
    LoadString(g_hInst, IDS_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL DeleteNotificationIcon()
{
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_GUID;
    nid.guidItem = PrinterIconGUID;
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL ShowLowInkBalloon()
{
    // Display a low ink balloon message. This is a warning, so show the appropriate system icon.
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_INFO | NIF_GUID;
    nid.guidItem = PrinterIconGUID;
    // respect quiet time since this balloon did not come from a direct user action.
    nid.dwInfoFlags = NIIF_WARNING | NIIF_RESPECT_QUIET_TIME;
    LoadString(g_hInst, IDS_LOWINK_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    LoadString(g_hInst, IDS_LOWINK_TEXT, nid.szInfo, ARRAYSIZE(nid.szInfo));
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL ShowNoInkBalloon()
{
    // Display an out of ink balloon message. This is a error, so show the appropriate system icon.
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_INFO | NIF_GUID;
    nid.guidItem = PrinterIconGUID;
    nid.dwInfoFlags = NIIF_ERROR;
    LoadString(g_hInst, IDS_NOINK_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    LoadString(g_hInst, IDS_NOINK_TEXT, nid.szInfo, ARRAYSIZE(nid.szInfo));
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL ShowPrintJobBalloon()
{
    // Display a balloon message for a print job with a custom icon
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_INFO | NIF_GUID;
    nid.guidItem = PrinterIconGUID;
    nid.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
    LoadString(g_hInst, IDS_PRINTJOB_TITLE, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    LoadString(g_hInst, IDS_PRINTJOB_TEXT, nid.szInfo, ARRAYSIZE(nid.szInfo));
    LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI_NOTIFICATIONICON), LIM_LARGE, &nid.hBalloonIcon);
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL RestoreTooltip()
{
    // After the balloon is dismissed, restore the tooltip.
    NOTIFYICONDATA nid = {sizeof(nid)};
    nid.uFlags = NIF_SHOWTIP | NIF_GUID;
    nid.guidItem = PrinterIconGUID;
    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void PositionFlyout(HWND hwnd, REFGUID guidIcon)
{
    // find the position of our printer icon
    NOTIFYICONIDENTIFIER nii = {sizeof(nii)};
    nii.guidItem = guidIcon;
    RECT rcIcon;
    HRESULT hr = Shell_NotifyIconGetRect(&nii, &rcIcon);
    if (SUCCEEDED(hr))
    {
        // display the flyout in an appropriate position close to the printer icon
        POINT const ptAnchor = { (rcIcon.left + rcIcon.right) / 2, (rcIcon.top + rcIcon.bottom)/2 };

        RECT rcWindow;
        GetWindowRect(hwnd, &rcWindow);
        SIZE sizeWindow = {rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top};

        if (CalculatePopupWindowPosition(&ptAnchor, &sizeWindow, TPM_VERTICAL | TPM_VCENTERALIGN | TPM_CENTERALIGN | TPM_WORKAREA, &rcIcon, &rcWindow))
        {
            // position the flyout and make it the foreground window
            SetWindowPos(hwnd, HWND_TOPMOST, rcWindow.left, rcWindow.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        }
    }
}

HWND ShowFlyout(HWND hwndMainWindow)
{
    if (g_pMMDeviceController)
    {
        g_pMMDeviceController->dump();
    }

    // size of the bitmap image (which will be the client area of the flyout window).
    RECT rcWindow = {};
    rcWindow.right = 214;
    rcWindow.bottom = 180;
    DWORD const dwStyle = WS_POPUP | WS_THICKFRAME;
    // adjust the window size to take the frame into account
    AdjustWindowRectEx(&rcWindow, dwStyle, FALSE, WS_EX_TOOLWINDOW);

    HWND hwndFlyout = CreateWindowEx(WS_EX_TOOLWINDOW, szFlyoutWindowClass, NULL, dwStyle,
        CW_USEDEFAULT, 0, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, hwndMainWindow, NULL, g_hInst, NULL);
    if (hwndFlyout)
    {
        PositionFlyout(hwndFlyout, PrinterIconGUID);
        SetForegroundWindow(hwndFlyout);
    }
    return hwndFlyout;
}

void HideFlyout(HWND hwndMainWindow, HWND hwndFlyout)
{
    DestroyWindow(hwndFlyout);

    // immediately after hiding the flyout we don't want to allow showing it again, which will allow clicking
    // on the icon to hide the flyout. If we didn't have this code, clicking on the icon when the flyout is open
    // would cause the focus change (from flyout to the taskbar), which would trigger hiding the flyout
    // (see the WM_ACTIVATE handler). Since the flyout would then be hidden on click, it would be shown again instead
    // of hiding.
    SetTimer(hwndMainWindow, HIDEFLYOUT_TIMER_ID, GetDoubleClickTime(), NULL);
}

void ShowContextMenu(HWND hwnd, POINT pt)
{
    HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenu)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
            {
                uFlags |= TPM_RIGHTALIGN;
            }
            else
            {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}


//HANDLE open_device(const wchar_t* path, BOOL open_rw)
//{
//    HANDLE handle = NULL;
//    DWORD desired_access = (open_rw) ? (GENERIC_WRITE | GENERIC_READ) : 0;
//    DWORD share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
//
//    handle = CreateFileW(path,
//        desired_access,
//        share_mode,
//        NULL,
//        OPEN_EXISTING,
//        FILE_FLAG_OVERLAPPED,/*FILE_ATTRIBUTE_NORMAL,*/
//        0);
//
//    if (handle == INVALID_HANDLE_VALUE)
//    {
//        return NULL;
//    }
//
//    return handle;
//}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND s_hwndFlyout = NULL;
    static BOOL s_fCanShowFlyout = TRUE;

    for (auto iter = g_WMAPPMessageHanders.begin();
        iter != g_WMAPPMessageHanders.end();
        ++iter)
    {
        WMAPPMessageHandler msgHandler = *iter;
        if (msgHandler.handler(message, wParam, lParam))
        {
            return 0;
        }
    }

    switch (message)
    {
    case WM_CREATE:
        // add the notification icon
        if (!AddNotificationIcon(hwnd))
        {
            MessageBox(hwnd,
                L"Please read the ReadMe.txt file for troubleshooting",
                L"Error adding icon", MB_OK);
            return -1;
        }
        break;
    case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_LOWINK:
                ShowLowInkBalloon();
                break;

            case IDM_NOINK:
                ShowNoInkBalloon();
                break;

            case IDM_PRINTJOB:
                ShowPrintJobBalloon();
                break;

            case IDM_OPTIONS:
                // placeholder for an options dialog
                //MessageBox(hwnd,  L"Display the options dialog here.", L"Options", MB_OK);
                ShowWindow(hwnd, SW_SHOW);
                break;

            // Mone: add a Hide command to hide the main window
            case IDM_HIDE:
                ShowWindow(hwnd, SW_HIDE);
                break;

            case IDM_EXIT:
                DestroyWindow(hwnd);
                break;

            case IDM_FLYOUT:
                s_hwndFlyout = ShowFlyout(hwnd);
                break;

            case IDM_REFRESH_AUDIO:
            {
                CMMDeviceController* pController = g_pMMDeviceController;
                pController->Refresh();
                break;
            }

            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }
        break;

    case WMAPP_NOTIFYCALLBACK:
        switch (LOWORD(lParam))
        {
        case NIN_SELECT:
            // for NOTIFYICON_VERSION_4 clients, NIN_SELECT is prerable to listening to mouse clicks and key presses
            // directly.
            if (IsWindowVisible(s_hwndFlyout))
            {
                HideFlyout(hwnd, s_hwndFlyout);
                s_hwndFlyout = NULL;
                s_fCanShowFlyout = FALSE;
            }
            else if (s_fCanShowFlyout)
            {
                s_hwndFlyout = ShowFlyout(hwnd);
            }
            break;

        case NIN_BALLOONTIMEOUT:
            RestoreTooltip();
            break;

        case NIN_BALLOONUSERCLICK:
            RestoreTooltip();
            // placeholder for the user clicking on the balloon.
            MessageBox(hwnd, L"The user clicked on the balloon.", L"User click", MB_OK);
            break;

        case WM_CONTEXTMENU:
            {
                POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
                ShowContextMenu(hwnd, pt);
            }
            break;
        }
        break;

    case WMAPP_HIDEFLYOUT:
        HideFlyout(hwnd, s_hwndFlyout);
        s_hwndFlyout = NULL;
        s_fCanShowFlyout = FALSE;
        break;

    case WMAPP_REFRESH_SESSION:
    {
        RefreshSessionData* pRefreshData = (RefreshSessionData*)wParam;
        if (pRefreshData)
        {
            g_pMMDeviceController->RefreshSession(pRefreshData->sessionID);
            delete pRefreshData;
        }
        break;
    }

    case WMAPP_REFRESH_DEVICE_SESSIONS:
    {
        RefreshDeviceData* pRefreshData = (RefreshDeviceData*)wParam;
        if (pRefreshData)
        {
            g_pMMDeviceController->RefreshDeviceSessions(pRefreshData->deviceID);
            delete pRefreshData;
        }
        break;
    }

    case WMAPP_REFRESH_DEVICE_PROPERTIES:
    {
        RefreshDeviceData* pRefreshData = (RefreshDeviceData*)wParam;
        if (pRefreshData)
        {
            g_pMMDeviceController->RefreshDeviceProperties(pRefreshData->deviceID);
            delete pRefreshData;
        }
        break;
    }

    case WMAPP_REFRESH_AUDIOCONTROLLER:
    {
        RefreshControllerData* pRefreshData = (RefreshControllerData*)wParam;
        if (pRefreshData)
        {
            g_pMMDeviceController->Refresh();
            delete pRefreshData;
        }
        break;
    }

    //case WM_DEVICECHANGE:
    //{
    //    switch (wParam)
    //    {
    //    //case DBT_DEVNODES_CHANGED:
    //    //{
    //    //    ATLTRACE("DBT_DEVNODES_CHANGED\n");
    //    //    break;
    //    //}

    //    case DBT_DEVICEARRIVAL:
    //    {
    //        ATLTRACE("DBT_DEVICEARRIVAL\n");
    //        DEV_BROADCAST_HDR* pDevHDR = (DEV_BROADCAST_HDR*)lParam;
    //        if (pDevHDR->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
    //        {
    //            DEV_BROADCAST_DEVICEINTERFACE* pDevInterface = (DEV_BROADCAST_DEVICEINTERFACE*)pDevHDR;
    //            ATLTRACE(TEXT("    dbcc_name: %s\n"), pDevInterface->dbcc_name);

    //            HANDLE handle = open_device(pDevInterface->dbcc_name, TRUE);
    //            if (handle != NULL)
    //            {
    //                CloseHandle(handle);
    //            }
    //        }

    //        break;
    //    }

    //    case DBT_DEVICEREMOVECOMPLETE:
    //    {
    //        ATLTRACE("DBT_DEVICEREMOVECOMPLETE\n");
    //        DEV_BROADCAST_HDR* pDevHDR = (DEV_BROADCAST_HDR*)lParam;
    //        if (pDevHDR->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
    //        {
    //            DEV_BROADCAST_DEVICEINTERFACE* pDevInterface = (DEV_BROADCAST_DEVICEINTERFACE*)pDevHDR;
    //            ATLTRACE(TEXT("    dbcc_name: %s\n"), pDevInterface->dbcc_name);
    //        }
    //        break;
    //    }



    //    default:
    //    {
    //        break;
    //    }

    //    }
    //    break;
    //}

    case WM_TIMER:
        if (wParam == HIDEFLYOUT_TIMER_ID)
        {
            // please see the comment in HideFlyout() for an explanation of this code.
            KillTimer(hwnd, HIDEFLYOUT_TIMER_ID);
            s_fCanShowFlyout = TRUE;
        }
        break;

    case WM_DESTROY:
        DeleteNotificationIcon();
        PostQuitMessage(0);
        break;
    
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void FlyoutPaint(HWND hwnd, HDC hdc)
{
    // Since this is a DPI aware application (see DeclareDPIAware.manifest), if the flyout window
    // were to show text we would need to increase the size. We could also have multiple sizes of
    // the bitmap image and show the appropriate image for each DPI, but that would complicate the
    // sample.
    static HBITMAP hbmp = NULL;
    if (hbmp == NULL)
    {
        hbmp = (HBITMAP)LoadImage(g_hInst, MAKEINTRESOURCE(IDB_PRINTER), IMAGE_BITMAP, 0, 0, 0);
    }
    if (hbmp)
    {
        RECT rcClient;
        GetClientRect(hwnd, &rcClient);
        HDC hdcMem = CreateCompatibleDC(hdc);
        if (hdcMem)
        {
            HGDIOBJ hBmpOld = SelectObject(hdcMem, hbmp);
            BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hBmpOld);
            DeleteDC(hdcMem);
        }
    }
}
LRESULT CALLBACK FlyoutWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            // paint a pretty picture
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FlyoutPaint(hwnd, hdc);
            EndPaint(hwnd, &ps);
        }
        break;
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            // when the flyout window loses focus, hide it.
            PostMessage(GetParent(hwnd), WMAPP_HIDEFLYOUT, 0, 0);
        }
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
