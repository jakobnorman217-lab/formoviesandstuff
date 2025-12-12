#include "Fsr4Wrapper.h"

#include <assert.h>
#include <vector>

// TODO: Include FidelityFX backend/util headers for allocating resources and creating FSR4 contexts.
// #include "external/FidelityFX-SDK-2.1.0/include/ffx_fsr4.h"
// #include "external/FidelityFX-SDK-2.1.0/include/ffx_error.h"
// #include "external/FidelityFX-SDK-2.1.0/include/ffx_types.h"

// Notes:
// This wrapper centralizes all FSR-specific calls so the rest of the app only needs to provide
// a D3D11 texture for input, and receives a D3D11 texture/SRV for the upscaled output.

Fsr4Wrapper::~Fsr4Wrapper()
{
    // TODO: Destroy FSR4 context with actual API call, e.g. ffxFsrContextDestroy(m_fsrContext)
    m_fsrContext = nullptr;
}

bool Fsr4Wrapper::Initialize(const Fsr4InitParams &params)
{
    m_device = params.device;
    m_context = params.context;
    m_inputWidth = params.inputWidth;
    m_inputHeight = params.inputHeight;
    m_outputWidth = params.outputWidth;
    m_outputHeight = params.outputHeight;
    m_colorFormat = params.colorFormat;

    if (!CreateUpscaledOutput(params))
        return false;
    if (!CreateContext(params))
        return false;

    if (params.outputTexture)
        m_outputTexture.CopyTo(params.outputTexture);
    if (params.outputSRV)
        m_outputSRV.CopyTo(params.outputSRV);

    return true;
}

bool Fsr4Wrapper::CreateUpscaledOutput(const Fsr4InitParams &params)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = params.outputWidth;
    desc.Height = params.outputHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = params.colorFormat;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &m_outputTexture);
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    hr = m_device->CreateShaderResourceView(m_outputTexture.Get(), &srvDesc, &m_outputSRV);
    if (FAILED(hr))
        return false;

    return true;
}

bool Fsr4Wrapper::CreateContext(const Fsr4InitParams &params)
{
    // TODO: Translate these placeholders to actual FSR4 context creation.
    // Example pseudo-code based on common FFX patterns:
    // FfxInterface interface = {};
    // ffxGetInterfaceD3D11(&interface, params.device); // TODO: check actual helper function name
    // FfxFsrContextDescription desc = {};
    // desc.flags = FFX_FSR_ENABLE_HIGH_DYNAMIC_RANGE; // or similar flags
    // desc.maxRenderSize.width = params.inputWidth;
    // desc.maxRenderSize.height = params.inputHeight;
    // desc.displaySize.width = params.outputWidth;
    // desc.displaySize.height = params.outputHeight;
    // desc.colorFormat = params.colorFormat;
    // desc.device = interface; // depends on actual API
    // ffxFsrContextCreate(&m_fsrContext, &desc);

    // Until actual integration, just keep the pointer null and return true.
    m_fsrContext = reinterpret_cast<void*>(0x1); // Dummy non-null to indicate "initialized"
    return true;
}

bool Fsr4Wrapper::Dispatch(ID3D11Texture2D *inputColor)
{
    assert(inputColor);

    // TODO: replace with actual FSR4 dispatch code.
    // Example pseudo-code:
    // FfxCommandList commandList = ffxGetCommandListDX11(m_context);
    // FfxFsrDispatchDescription dispatch = {};
    // dispatch.color = ffxGetTextureResourceDX11(inputColor);
    // dispatch.output = ffxGetWritableTextureResourceDX11(m_outputTexture.Get());
    // dispatch.jitterOffset.x = 0; // TODO: hook to camera jitter if available
    // dispatch.jitterOffset.y = 0;
    // dispatch.sharpness = 0.2f; // TODO: expose parameter
    // ffxFsrContextDispatch(m_fsrContext, &dispatch);

    // For now, simply copy input to output as a placeholder.
    m_context->CopyResource(m_outputTexture.Get(), inputColor);
    return true;
}
