#pragma once
#include "stdafx.h"
#include "GameTimer.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

class D3D12App {
protected:
	explicit D3D12App(HINSTANCE _hInstance);
	virtual ~D3D12App();

public:
	virtual bool Initialize();
	virtual LRESULT MsgProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

	int RunWindow();

	static D3D12App* GetThis();

protected:
	virtual void BuildAllDescriptorHeaps();
	virtual void OnResize();
	virtual void Update(GameTimer& _gt) = 0;
	virtual void Draw() = 0;

	virtual void OnMouseDown(WPARAM _btnState, int _x, int _y) { }
	virtual void OnMouseUp(WPARAM _btnState, int _x, int _y)   { }
	virtual void OnMouseMove(WPARAM _btnState, int _x, int _y) { }
	virtual void OnKeyPressed(WPARAM _key) {}

	bool InitWindowApplication();

	void WaitForPreviousFrame();

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHeapCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVHeapGPUHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHeapCPUHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVHeapGPUHandle() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrBackBufferView() const;
	ID3D12Resource* GetCurrBackBuffer() const;
	HINSTANCE GetHINST() const;
	HWND GetHWND() const;
	float GetAspectRatio() const;

private:
	bool InitD3D12();
	bool InitD3D11();
	void BuildFence();
	void BuildCommandObjects();
	void BuildSwapChain();

	void CalculateFrame();

protected:
	HINSTANCE m_hInst = nullptr;
	HWND m_hWnd = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> m_device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence = nullptr;
	HANDLE m_fenceEvent = NULL;
	UINT64 m_currFenceValue = 0;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[FRAME] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;
	D3D12_VIEWPORT m_viewPort;
	D3D12_RECT m_scissorRect;
	UINT m_currBackBufferIndex = 0;

	Microsoft::WRL::ComPtr<ID3D11On12Device> m_d3d11On12Device = nullptr;
	Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext = nullptr;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext = nullptr;
	UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory = nullptr;
	Microsoft::WRL::ComPtr<IDWriteFactory> m_dWriteFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_wrappedBackBuffers[FRAME] = { nullptr };
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[FRAME] = { nullptr };
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
	UINT dxgiFactoryFlags = 0;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
	UINT m_RTVDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CBVSRVUAVDescSize = 0;

	bool m_isPaused = false;
	bool m_isMinimized = false;
	bool m_isMaximized = false;

	GameTimer m_timer;

private:
	static D3D12App* m_app;
#pragma region _Application Setting Value_
protected:
	UINT m_width = 1280;
	UINT m_height = 960;

	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bool m_useWarpDevice = false;

	std::wstring m_mainWndCaption = L"D3D12App";
#pragma endregion
#pragma region _Forbidden Constructor_
private:
	D3D12App() = delete;
	D3D12App(const D3D12App& _rhs) = delete;
	D3D12App& operator=(const D3D12App& _rhs) = delete;
#pragma endregion
};