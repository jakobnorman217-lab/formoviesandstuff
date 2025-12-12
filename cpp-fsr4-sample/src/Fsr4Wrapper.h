#pragma once

#include <wrl/client.h>
#include <d3d11.h>

// TODO: Include actual FidelityFX SDK headers (FSR4) here.
// e.g., #include "external/FidelityFX-SDK-2.1.0/include/ffx_fsr4.h"

struct Fsr4InitParams
{
    ID3D11Device *device = nullptr;
    ID3D11DeviceContext *context = nullptr;
    UINT inputWidth = 0;
    UINT inputHeight = 0;
    UINT outputWidth = 0;
    UINT outputHeight = 0;
    DXGI_FORMAT colorFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

    // Output resources will be created internally; we return them via out parameters.
    ID3D11Texture2D **outputTexture = nullptr;
    ID3D11ShaderResourceView **outputSRV = nullptr;
};

class Fsr4Wrapper
{
public:
    Fsr4Wrapper() = default;
    ~Fsr4Wrapper();

    bool Initialize(const Fsr4InitParams &params);
    bool Dispatch(ID3D11Texture2D *inputColor);

private:
    bool CreateUpscaledOutput(const Fsr4InitParams &params);
    bool CreateContext(const Fsr4InitParams &params);

private:
    ID3D11Device *m_device = nullptr; // weak
    ID3D11DeviceContext *m_context = nullptr; // weak

    UINT m_inputWidth = 0;
    UINT m_inputHeight = 0;
    UINT m_outputWidth = 0;
    UINT m_outputHeight = 0;
    DXGI_FORMAT m_colorFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_outputTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_outputSRV;

    // TODO: Replace placeholders with actual FSR4 context types.
    void *m_fsrContext = nullptr; // TODO: FfxFsrContext
};
