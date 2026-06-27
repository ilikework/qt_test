#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifndef WM_USER
#define WM_USER 0x0400
#endif

/* lib.h 用 HWND 上报分析进度；独立 DLL 中改为空操作 */
inline LRESULT LibFA_SendMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd;
    (void)msg;
    (void)wParam;
    (void)lParam;
    return 0;
}
