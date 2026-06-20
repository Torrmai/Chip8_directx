#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>
using Microsoft::WRL::ComPtr;

class ScreenInterface
{
public:
	bool Init(HWND hwnd, int width, int height);
	void Render(const uint32_t* pixels, int pixelWidth, int pixelHeight);
	void Present();

private:
    ComPtr<ID3D11Device>           device;
    ComPtr<ID3D11DeviceContext>    context;
    ComPtr<IDXGISwapChain>         swapChain;
    ComPtr<ID3D11RenderTargetView> rtv;

    ComPtr<ID3D11Texture2D>          screenTexture;
    ComPtr<ID3D11ShaderResourceView> textureView;
    ComPtr<ID3D11SamplerState>       sampler;
    ComPtr<ID3D11VertexShader>       vertexShader;
    ComPtr<ID3D11PixelShader>        pixelShader;
    ComPtr<ID3D11InputLayout>        inputLayout;
    ComPtr<ID3D11Buffer>             vertexBuffer;

    bool CreateDeviceAndSwapChain(HWND hwnd, int width, int height);
    bool CreateTexture(int pixelWidth, int pixelHeight);
    bool CreateShaders();
    bool CreateFullscreenQuad();
};
