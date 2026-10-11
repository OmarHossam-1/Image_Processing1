// Renderer.h - DX11 image preview texture management
#pragma once
#include <d3d11.h>

struct AppState;

// Upload the current working image from AppState into a DX11 shader resource view.
// Converts RGB -> RGBA (alpha=255) for DXGI_FORMAT_R8G8B8A8_UNORM.
// Releases the previous SRV before creating a new one.
void UpdatePreviewTexture(AppState& state,
                          ID3D11Device*        device,
                          ID3D11DeviceContext* ctx);

// Release the preview SRV stored in state (called on shutdown or when needed).
void ReleasePreviewTexture(AppState& state);

// Create a generic SRV from any Image object
ID3D11ShaderResourceView* CreateTextureFromImage(const Image& img, ID3D11Device* device);
