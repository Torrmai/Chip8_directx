#include "ScreenInterface.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

bool ScreenInterface::Init(HWND hwnd, int width, int height)
{
    if (!CreateDeviceAndSwapChain(hwnd, width, height)) return false;
    if (!CreateTexture(64, 32)) return false;       // CHIP-8's native resolution
    if (!CreateShaders()) return false;
    if (!CreateFullscreenQuad()) return false;
    return true;
}

bool ScreenInterface::CreateDeviceAndSwapChain(HWND hwnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &swapChain, &device, nullptr, &context);

    if (FAILED(hr)) return false;

    ComPtr<ID3D11Texture2D> backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv);
    if (FAILED(hr)) return false;

    D3D11_VIEWPORT vp{};
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);

    return true;
}

bool ScreenInterface::CreateTexture(int pixelWidth, int pixelHeight)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = pixelWidth;
    desc.Height = pixelHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &screenTexture);
    if (FAILED(hr)) return false;

    hr = device->CreateShaderResourceView(screenTexture.Get(), nullptr, &textureView);
    if (FAILED(hr)) return false;

    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // nearest-neighbor, keeps pixels crisp when scaled up
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    hr = device->CreateSamplerState(&sampDesc, &sampler);
    if (FAILED(hr)) return false;

    return true;
}

bool ScreenInterface::CreateShaders()
{
    static const char* shaderSrc = R"(
        struct VOut
        {
            float4 pos : SV_POSITION;
            float2 uv  : TEXCOORD;
        };

        VOut VSMain(float4 pos : POSITION, float2 uv : TEXCOORD)
        {
            VOut output;
            output.pos = pos;
            output.uv = uv;
            return output;
        }

        Texture2D tex : register(t0);
        SamplerState samp : register(s0);

        float4 PSMain(VOut input) : SV_Target
        {
            return tex.Sample(samp, input.uv);
        }
    )";

    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    HRESULT hr = D3DCompile(shaderSrc, strlen(shaderSrc), nullptr, nullptr, nullptr,
        "VSMain", "vs_4_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = D3DCompile(shaderSrc, strlen(shaderSrc), nullptr, nullptr, nullptr,
        "PSMain", "ps_4_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
    if (FAILED(hr)) return false;

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr)) return false;

    return true;
}

bool ScreenInterface::CreateFullscreenQuad()
{
    // Each vertex: x, y (clip space, -1..1), u, v (texture coords, 0..1)
    // Triangle strip covering the whole screen
    float quadVerts[] =
    {
        -1.0f,  1.0f,  0.0f, 0.0f, // top-left
         1.0f,  1.0f,  1.0f, 0.0f, // top-right
        -1.0f, -1.0f,  0.0f, 1.0f, // bottom-left
         1.0f, -1.0f,  1.0f, 1.0f, // bottom-right
    };

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(quadVerts);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = quadVerts;

    HRESULT hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    return SUCCEEDED(hr);
}

void ScreenInterface::Render(const uint32_t* pixels, int pixelWidth, int pixelHeight)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(screenTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    uint8_t* dst = (uint8_t*)mapped.pData;
    for (int row = 0; row < pixelHeight; row++)
        memcpy(dst + row * mapped.RowPitch, pixels + row * pixelWidth, pixelWidth * 4);

    context->Unmap(screenTexture.Get(), 0);

    float clearColor[4] = { 0, 0, 0, 1 };
    context->ClearRenderTargetView(rtv.Get(), clearColor);
    context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);

    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->PSSetShaderResources(0, 1, textureView.GetAddressOf());
    context->PSSetSamplers(0, 1, sampler.GetAddressOf());
    context->IASetInputLayout(inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    UINT stride = sizeof(float) * 4, offset = 0;
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->Draw(4, 0); // fullscreen quad, 4 verts as a triangle strip
}

void ScreenInterface::Present()
{
    swapChain->Present(1, 0);
}