// Renderer.cpp - DX11 preview texture management for Pixel Lab
#include "pch.h"
#include <d3d11.h>
#include <vector>
#include "Renderer.h"
#include "AppState.h"

void ReleasePreviewTexture(AppState& state)
{
    if (state.previewSRV) {
        state.previewSRV->Release();
        state.previewSRV = nullptr;
    }
    state.texWidth  = 0;
    state.texHeight = 0;
}

void UpdatePreviewTexture(AppState& state,
                          ID3D11Device*        device,
                          ID3D11DeviceContext* /*ctx*/)
{
    ReleasePreviewTexture(state);

    if (!state.imageLoaded || !state.working.imageData ||
        state.working.width <= 0 || state.working.height <= 0)
        return;

    const int w = state.working.width;
    const int h = state.working.height;

    // Convert RGB (3-channel) to RGBA (4-channel, alpha=255)
    std::vector<unsigned char> rgba;
    rgba.reserve(static_cast<size_t>(w) * h * 4);
    const unsigned char* src = state.working.imageData;
    for (int i = 0; i < w * h; ++i) {
        rgba.push_back(src[i * 3 + 0]);  // R
        rgba.push_back(src[i * 3 + 1]);  // G
        rgba.push_back(src[i * 3 + 2]);  // B
        rgba.push_back(255);              // A
    }

    // Create D3D11 texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = static_cast<UINT>(w);
    desc.Height           = static_cast<UINT>(h);
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem     = rgba.data();
    initData.SysMemPitch = static_cast<UINT>(w * 4);

    ID3D11Texture2D* tex = nullptr;
    if (FAILED(device->CreateTexture2D(&desc, &initData, &tex)))
        return;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    device->CreateShaderResourceView(tex, &srvDesc, &state.previewSRV);
    tex->Release();

    state.texWidth  = w;
    state.texHeight = h;
}

ID3D11ShaderResourceView* CreateTextureFromImage(const Image& img, ID3D11Device* device)
{
    if (!img.imageData || img.width <= 0 || img.height <= 0) return nullptr;

    const int w = img.width;
    const int h = img.height;

    std::vector<unsigned char> rgba;
    rgba.reserve(static_cast<size_t>(w) * h * 4);
    const unsigned char* src = img.imageData;
    for (int i = 0; i < w * h; ++i) {
        rgba.push_back(src[i * 3 + 0]);
        rgba.push_back(src[i * 3 + 1]);
        rgba.push_back(src[i * 3 + 2]);
        rgba.push_back(255);
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = static_cast<UINT>(w);
    desc.Height           = static_cast<UINT>(h);
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem     = rgba.data();
    initData.SysMemPitch = static_cast<UINT>(w * 4);

    ID3D11Texture2D* tex = nullptr;
    if (FAILED(device->CreateTexture2D(&desc, &initData, &tex))) return nullptr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    ID3D11ShaderResourceView* srv = nullptr;
    device->CreateShaderResourceView(tex, &srvDesc, &srv);
    tex->Release();

    return srv;
}
