#pragma once

#include "stdafx.h"

class RayTracingApp
{
public:
	virtual void Initialize(HINSTANCE instanceHandle, int show);
	virtual int Run() noexcept;

protected:
	virtual void OnInitialize(HWND hWnd) = 0;
	virtual void OnRender(HWND hWnd) = 0;
	virtual int GetBackBufferWidth() const = 0;
	virtual int GetBackBufferHeight() const = 0;
	virtual std::wstring GetWindowName() const = 0;

private:
	void InitDirect2D(HWND hWnd) noexcept;
	void InitBuffers();

protected:
	Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Bitmap> m_backbufferBitmap;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

	std::vector<DirectX::XMVECTOR> m_backbufferHdr;
	std::vector<DirectX::PackedVector::XMCOLOR> m_backbufferLdr;

	HWND m_wndHandle;
};
