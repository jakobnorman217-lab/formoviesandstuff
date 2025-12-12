#include "App.h"

#include <windowsx.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    AppState state = {};
    state.hInstance = hInstance;

    const int initialWidth = 1280;
    const int initialHeight = 720;

    if (!InitWindow(state, L"FSR4 Desktop Upscaler", initialWidth, initialHeight))
        return -1;
    if (!InitD3D(state))
        return -2;
    if (!InitDesktopDuplication(state))
        return -3;
    if (!InitFullscreenTriangleShaders(state))
        return -4;
    if (!InitSampler(state))
        return -5;

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (!CaptureFrame(state))
            break;

        if (!UpscaleFrame(state))
            break;

        RenderUpscaled(state);
    }

    Cleanup(state);
    return 0;
}
