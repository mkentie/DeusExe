#include "stdafx.h"
#include "RawInput.h"

//From hidusage.h
typedef USHORT USAGE, *PUSAGE;
#define HID_USAGE_GENERIC_MOUSE ((USAGE) 0x02)
#define HID_USAGE_PAGE_GENERIC ((USAGE) 0x01)

bool RegisterRawInput( const HWND hWnd )
{
    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid[0].dwFlags = 0;
    Rid[0].hwndTarget = hWnd;
    return RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))!=0;
}
