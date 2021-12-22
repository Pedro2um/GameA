/*
    This game is developed only for educational purpose,
    it is a "follow along" code, from a youtube series on
    game devlopment with the C programming language.

*/

#include <stdio.h>

#pragma warning(push, 0)

#include <windows.h>

#pragma warning(pop)

#include <stdint.h>

#include "Main.h"

//#pragma warning(disable: 5045) // Disable warning about Spectre/Meltdown CPU vulnerability

//#pragma warning(disable: 4668) // Disable warning about macros of windows.h




HWND gGameWindow;

BOOL gGameIsRunning;

GAMEBITMAP gBackBuffer;

GAMEPERFDATA   gPerformaceData;

#pragma warning(disable: 28251)

int __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CmdShow)
{   
    

    
    UNREFERENCED_PARAMETER(Instance);

	UNREFERENCED_PARAMETER(PreviousInstance);

	UNREFERENCED_PARAMETER(CommandLine);

	UNREFERENCED_PARAMETER(CmdShow);

    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBoxA(
            NULL, 
            "Another instance of this program is already running!",
            "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) 
    {
        
        goto Exit;
    }

    QueryPerformanceFrequency(&gPerformaceData.PerfFrequency);
    

    gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);

    gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;

    gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;

    gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

    gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;

    gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;


    gBackBuffer.Memory = VirtualAlloc(
        NULL,
        GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);

    if (gBackBuffer.Memory == NULL)
    {
        MessageBoxA(
            NULL, 
            "Failed to allocate memory for drawing surface!",
            "Error!",
            MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    memset(gBackBuffer.Memory, 0x7F, GAME_DRAWING_AREA_MEMORY_SIZE);

    MSG Message = { 0 };

    gGameIsRunning = TRUE;

    while (TRUE == gGameIsRunning )
    {   
        QueryPerformanceCounter(&gPerformaceData.FrameStart);

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();


        QueryPerformanceCounter(&gPerformaceData.FrameEnd);

        gPerformaceData.ElapsedMicrosecondsPerFrame.QuadPart = gPerformaceData.FrameEnd.QuadPart - gPerformaceData.FrameStart.QuadPart;

        gPerformaceData.ElapsedMicrosecondsPerFrame.QuadPart *= 1000000;
        gPerformaceData.ElapsedMicrosecondsPerFrame.QuadPart /= gPerformaceData.PerfFrequency.QuadPart;

        Sleep(1);

        gPerformaceData.TotalFramesRendered++;

        if ( (gPerformaceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES) == 0)
        {   
            char str[64] = { 0 };
            _snprintf_s(
                str,
                _countof(str),
                _TRUNCATE,
                "Elapsed microseconds: %lli\n",
                gPerformaceData.ElapsedMicrosecondsPerFrame.QuadPart);
            OutputDebugStringA(str);
        }

    }

  



Exit:

	return (0);
} 


LRESULT CALLBACK MainWindowdProc(
    _In_ HWND WindowHandle, 
    _In_ UINT Message, 
    _In_ WPARAM WParam, 
    _In_ LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
        case WM_CLOSE:
        {   

            gGameIsRunning = FALSE;

            PostQuitMessage(0);

            break;
        }

        default:
        {
           Result =  DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }
    return (Result);
}

DWORD CreateMainGameWindow(void)
{
    DWORD Result = ERROR_SUCCESS;


    WNDCLASSEXA WindowClass = { 0 };

    WindowClass.cbSize = sizeof(WNDCLASSEXA);

    WindowClass.style = 0;

    WindowClass.lpfnWndProc = MainWindowdProc;

    WindowClass.cbClsExtra = 0;

    WindowClass.cbWndExtra = 0;

    WindowClass.hInstance = GetModuleHandleA(NULL); // GetModuleHandle(NULL) also works

    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


    if (RegisterClassExA(&WindowClass) == 0)
    {   
        Result = GetLastError();
        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }
    
    gGameWindow = CreateWindowExA(
        0,
        WindowClass.lpszClassName, 
        "Window Title",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT, 
        640, 
        480, 
        NULL,
        NULL, 
        GetModuleHandleA(NULL), NULL);

    if (gGameWindow == NULL)
    {   
        Result = GetLastError();
        MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        
        goto Exit;
    }

    gPerformaceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformaceData.MonitorInfo) == 0)
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto Exit;
    }

    gPerformaceData.MonitorWidth = gPerformaceData.MonitorInfo.rcMonitor.right - gPerformaceData.MonitorInfo.rcMonitor.left;

    gPerformaceData.MonitorHeight = gPerformaceData.MonitorInfo.rcMonitor.bottom - gPerformaceData.MonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(
        gGameWindow,
        GWL_STYLE, 
        (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

    if (SetWindowPos(gGameWindow, 
        HWND_TOP, 
        gPerformaceData.MonitorInfo.rcMonitor.left,
        gPerformaceData.MonitorInfo.rcMonitor.top,
        gPerformaceData.MonitorWidth, gPerformaceData.MonitorHeight,
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

    

 Exit:

    return(Result);
}

BOOL GameIsAlreadyRunning(void)
{
    HANDLE Mutex = NULL;

    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");

    if ( GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }

}

void ProcessPlayerInput(void)
{
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE); 

    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
}

void RenderFrameGraphics(void)
{   
    //memset(gBackBuffer.Memory, 0xFF, GAME_DRAWING_AREA_MEMORY_SIZE); 
    // the game will crash if you put 1 more byte than the macro GAME_DRAWING_AREA_MEMORY_SIZE

    PIXEL32 Pixel = { 0 };

    Pixel.Blue = 0x7f;

    Pixel.Green = 0;

    Pixel.Red = 0;

    Pixel.Alpha = 0xff;

    for (int x = 0; x < GAME_RES_WIDTH * GAME_RES_HEIGHT; x++)
    {
        memcpy_s((PIXEL32*)gBackBuffer.Memory + x, sizeof(PIXEL32), &Pixel, sizeof(PIXEL32) );
    }
        

    HDC DeviceContext = GetDC(gGameWindow);
    
    StretchDIBits(DeviceContext, 
        0, 
        0, 
        gPerformaceData.MonitorWidth,
        gPerformaceData.MonitorHeight,
        0, 
        0, 
        GAME_RES_WIDTH, 
        GAME_RES_HEIGHT,
        gBackBuffer.Memory, 
        &gBackBuffer.BitmapInfo,
        DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(gGameWindow, DeviceContext);
}