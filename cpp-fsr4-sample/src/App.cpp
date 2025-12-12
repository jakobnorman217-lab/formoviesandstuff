#include "App.h"

#include <assert.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <string>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    // Simple helper macro for HRESULT checking during prototyping.
    #define CHECK_HR(hr_call)                                      \
        do                                                          \
        {                                                           \
            HRESULT _hr = (hr_call);                                \
            if (FAILED(_hr))                                        \
            {                                                       \
                assert(false && "HRESULT failure");                \
                return false;                                       \
            }                                                       \
        } while (0)

    const UINT kFrameTimeoutMs = 16; // ~60fps

    const char *kFullscreenTriangleVS = R"( 
    struct VSOutput
    {
        float4 pos : SV_POSITION;
        float2 uv  : TEXCOORD0;
    };

    VSOutput main(uint vid : SV_VertexID)
    {
        float2 verts[3] = { float2(-1.0, -1.0), float2(-1.0, 3.0), float2(3.0, -1.0) };
        float2 uv[3] = { float2(0.0, 1.0), float2(0.0, -1.0), float2(2.0, 1.0) };

        VSOutput o;
        o.pos = float4(verts[vid], 0.0, 1.0);
        o.uv = uv[vid];
        return o;
    }
    )";

    const char *kBlitUpscaledPS = R"(
    Texture2D inputTex : register(t0);
    SamplerState linearSampler : register(s0);

    float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
    {
        return inputTex.Sample(linearSampler, uv);
    }
    )";
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool InitWindow(AppState &state, const wchar_t *title, int width, int height)
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = state.hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"FSR4SampleWindow";

    if (!RegisterClassEx(&wc))
        return false;

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    state.hwnd = CreateWindowEx(0, wc.lpszClassName, title, WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                rect.right - rect.left, rect.bottom - rect.top,
                                nullptr, nullptr, state.hInstance, nullptr);
    if (!state.hwnd)
        return false;

    ShowWindow(state.hwnd, SW_SHOWDEFAULT);
    UpdateWindow(state.hwnd);
    return true;
}

bool InitD3D(AppState &state)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = state.hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createFlags = 0;
#if defined(_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &scd,
        &swapChain,
        &device,
        nullptr,
        &context);
    if (FAILED(hr))
    {
        return false;
    }

    state.swapChain = swapChain;
    state.device = device;
    state.context = context;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    CHECK_HR(state.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
    CHECK_HR(state.device->CreateRenderTargetView(backBuffer.Get(), nullptr, &state.rtv));

    DXGI_SWAP_CHAIN_DESC desc = {};
    state.swapChain->GetDesc(&desc);
    state.backbufferWidth = desc.BufferDesc.Width;
    state.backbufferHeight = desc.BufferDesc.Height;

    return true;
}

bool InitDesktopDuplication(AppState &state)
{
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    CHECK_HR(state.device.As(&dxgiDevice));

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    CHECK_HR(dxgiDevice->GetAdapter(&adapter));

    Microsoft::WRL::ComPtr<IDXGIOutput> output;
    CHECK_HR(adapter->EnumOutputs(0, &output)); // Primary output

    Microsoft::WRL::ComPtr<IDXGIOutput1> output1;
    CHECK_HR(output.As(&output1));

    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication;
    CHECK_HR(output1->DuplicateOutput(state.device.Get(), &duplication));
    state.outputDuplication = duplication;

    // Determine desktop size from output.
    DXGI_OUTPUT_DESC outputDesc = {};
    output->GetDesc(&outputDesc);
    RECT desktopRect = outputDesc.DesktopCoordinates;
    UINT desktopWidth = desktopRect.right - desktopRect.left;
    UINT desktopHeight = desktopRect.bottom - desktopRect.top;

    D3D11_TEXTURE2D_DESC captureDesc = {};
    captureDesc.Width = desktopWidth;
    captureDesc.Height = desktopHeight;
    captureDesc.MipLevels = 1;
    captureDesc.ArraySize = 1;
    captureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    captureDesc.SampleDesc.Count = 1;
    captureDesc.Usage = D3D11_USAGE_DEFAULT;
    captureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    captureDesc.CPUAccessFlags = 0;

    CHECK_HR(state.device->CreateTexture2D(&captureDesc, nullptr, &state.capturedTexture));

    return true;
}

bool InitFullscreenTriangleShaders(AppState &state)
{
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(kFullscreenTriangleVS, strlen(kFullscreenTriangleVS), nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA(errorBlob ? (const char*)errorBlob->GetBufferPointer() : "Failed to compile VS" );
        return false;
    }

    hr = D3DCompile(kBlitUpscaledPS, strlen(kBlitUpscaledPS), nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA(errorBlob ? (const char*)errorBlob->GetBufferPointer() : "Failed to compile PS");
        return false;
    }

    CHECK_HR(state.device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &state.fullscreenVS));
    CHECK_HR(state.device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &state.blitPS));
    return true;
}

bool InitSampler(AppState &state)
{
    D3D11_SAMPLER_DESC desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;

    CHECK_HR(state.device->CreateSamplerState(&desc, &state.linearSampler));
    return true;
}

bool CaptureFrame(AppState &state)
{
    if (!state.outputDuplication)
        return false;

    DXGI_OUTDUPL_FRAME_INFO frameInfo = {};
    Microsoft::WRL::ComPtr<IDXGIResource> desktopResource;
    HRESULT hr = state.outputDuplication->AcquireNextFrame(kFrameTimeoutMs, &frameInfo, &desktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        return true; // No new frame yet
    }
    else if (FAILED(hr))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> acquiredTex;
    CHECK_HR(desktopResource.As(&acquiredTex));

    // Copy to our texture with shader resource bind flags.
    state.context->CopyResource(state.capturedTexture.Get(), acquiredTex.Get());

    CHECK_HR(state.outputDuplication->ReleaseFrame());
    return true;
}

bool UpscaleFrame(AppState &state)
{
    if (!state.fsr)
    {
        state.fsr = std::make_unique<Fsr4Wrapper>();
        Fsr4InitParams params = {};
        params.device = state.device.Get();
        params.context = state.context.Get();
        params.inputWidth = 0;  // Will be filled from captured texture
        params.inputHeight = 0;
        params.outputWidth = state.backbufferWidth;
        params.outputHeight = state.backbufferHeight;
        params.colorFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

        // Query captured texture size
        D3D11_TEXTURE2D_DESC desc = {};
        state.capturedTexture->GetDesc(&desc);
        params.inputWidth = desc.Width;
        params.inputHeight = desc.Height;

        params.outputTexture = state.upscaledTexture.GetAddressOf();
        params.outputSRV = state.upscaledSRV.GetAddressOf();

        if (!state.fsr->Initialize(params))
        {
            return false;
        }
    }

    return state.fsr->Dispatch(state.capturedTexture.Get());
}

void RenderUpscaled(AppState &state)
{
    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    state.context->OMSetRenderTargets(1, state.rtv.GetAddressOf(), nullptr);
    state.context->ClearRenderTargetView(state.rtv.Get(), clearColor);

    ID3D11ShaderResourceView *srvs[] = { state.upscaledSRV.Get() };
    state.context->PSSetShaderResources(0, 1, srvs);
    state.context->PSSetSamplers(0, 1, state.linearSampler.GetAddressOf());

    state.context->VSSetShader(state.fullscreenVS.Get(), nullptr, 0);
    state.context->PSSetShader(state.blitPS.Get(), nullptr, 0);

    state.context->IASetInputLayout(nullptr);
    state.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    state.context->Draw(3, 0);

    state.swapChain->Present(1, 0);
}

void Cleanup(AppState &state)
{
    state.fsr.reset();
    state.rtv.Reset();
    state.swapChain.Reset();
    state.context.Reset();
    state.device.Reset();
}
