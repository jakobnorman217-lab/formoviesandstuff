#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>
#include <string>

#include "Fsr4Wrapper.h"

struct AppState
{
    HINSTANCE hInstance = nullptr;
    HWND hwnd = nullptr;
    UINT backbufferWidth = 0;
    UINT backbufferHeight = 0;

    // D3D11 core
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;

    // Desktop duplication
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> outputDuplication;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> capturedTexture;

    // Upscaling
    Microsoft::WRL::ComPtr<ID3D11Texture2D> upscaledTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> upscaledSRV;
    std::unique_ptr<Fsr4Wrapper> fsr;

    // Rendering resources
    Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSampler;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> fullscreenVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> blitPS;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool InitWindow(AppState &state, const wchar_t *title, int width, int height);
bool InitD3D(AppState &state);
bool InitDesktopDuplication(AppState &state);
bool InitFullscreenTriangleShaders(AppState &state);
bool InitSampler(AppState &state);

bool CaptureFrame(AppState &state);
bool UpscaleFrame(AppState &state);
void RenderUpscaled(AppState &state);

void Cleanup(AppState &state);
