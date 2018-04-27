#pragma once

namespace AppSettings
{
	constexpr int k_backbufferWidth = 1280;
	constexpr int k_backbufferHeight = 720; 
}

class RayTracingApp
{
public:
	void OnInitialize(HWND hWnd);
	void OnRender(HWND hWnd);

private:
	void InitDirect2D(HWND hWnd);
	void InitBuffers();
	void DrawBitmap();

private:
	Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backbufferBitmap;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

	std::vector<DirectX::XMFLOAT3> m_backbufferHdr;
	std::vector<DirectX::PackedVector::XMCOLOR> m_backbufferLdr;
};
