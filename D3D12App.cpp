#include "stdafx.h"
#include "D3D12App.h"
#include "D3D12Helper.h"

#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "d3d11" )
#pragma comment(lib, "d2d1" )

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

LRESULT CALLBACK MainWndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam) {
	return D3D12App::GetThis()->MsgProc(_hwnd, _msg, _wParam, _lParam);
}

D3D12App::D3D12App(HINSTANCE _hInstance)
	: m_hInst(_hInstance) {
	assert(m_app == nullptr);
	m_app = this;
}

D3D12App::~D3D12App() {
	if (m_device != nullptr) {
		WaitForPreviousFrame();
		CloseHandle(m_fenceEvent);
	}
}

bool D3D12App::Initialize() {
	if (!InitWindowApplication()) { return false; }
	if (!InitD3D12()) { return false; }
	if (!InitD3D11()) { return false; }
	OnResize();

	return true;
}

LRESULT D3D12App::MsgProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam) {
	switch (_msg) {
	case WM_ACTIVATE:
		if (LOWORD(_wParam) == WA_INACTIVE) {

			m_isPaused = true;
			m_timer.Stop();
		}
		else {
			m_isPaused = true;
			m_timer.Start();
		}
		return 0;

	case WM_SIZE:
		m_width = LOWORD(_lParam);
		m_height = HIWORD(_lParam);
		if (m_device) {
			if (_wParam == SIZE_MINIMIZED) {
				m_isPaused = true;
				m_isMinimized = true;
				m_isMaximized = false;
			}
			else if (_wParam == SIZE_MAXIMIZED) {
				m_isPaused = false;
				m_isMinimized = false;
				m_isMaximized = true;
				OnResize();
			}
			else if (_wParam == SIZE_RESTORED) {
				if (m_isMinimized) {
					m_isPaused = false;
					m_isMinimized = false;
					OnResize();
				}
				else if (m_isMaximized) {
					m_isPaused = false;
					m_isMaximized = false;
					OnResize();
				}
				else {
					OnResize();
				}
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		m_isPaused = true;
		m_timer.Stop();
		return 0;

	case WM_EXITSIZEMOVE:
		m_isPaused = false;
		m_timer.Start();
		OnResize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)_lParam)->ptMinTrackSize.x = 1280;
		((MINMAXINFO*)_lParam)->ptMinTrackSize.y = 1000;
		((MINMAXINFO*)_lParam)->ptMaxTrackSize.x = 1280;
		((MINMAXINFO*)_lParam)->ptMaxTrackSize.y = 1000;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(_wParam, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(_wParam, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(_wParam, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam));
		return 0;

	case WM_KEYDOWN:
		if (_wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}

		OnKeyPressed(_wParam);
		return 0;
	}
	return DefWindowProc(_hWnd, _msg, _wParam, _lParam);
}

int D3D12App::RunWindow() {
	MSG msg = { 0 };

	m_timer.Reset();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_timer.Tick();

			if (!m_isPaused) {
				CalculateFrame();
				Update(m_timer);
				Draw();
			}
			else {
				Sleep(100);
			}
		}
	}
	return (int)msg.wParam;
}

void D3D12App::BuildAllDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void D3D12App::OnResize() {
	assert(m_device);
	assert(m_swapChain);
	assert(m_commandAllocator);

	WaitForPreviousFrame();

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	for (int i = 0; i < FRAME; ++i) {
		m_backBuffers[i].Reset();
	}

	m_depthStencilBuffer.Reset();
	m_currBackBufferIndex = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(GetRTVHeapCPUHandle());


	for (UINT i = 0; i < FRAME; ++i) {
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_RTVDescriptorSize);

		float dpiX;
		float dpiY;
		m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);

		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(m_backBuffers[i].Get(), &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&m_wrappedBackBuffers[i])));

		ComPtr<IDXGISurface> surface;
		ThrowIfFailed(m_wrappedBackBuffers[i].As(&surface));
		ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(surface.Get(), &bitmapProperties, &m_d2dRenderTargets[i]));
	}

	{
		D3D12_RESOURCE_DESC depthStencilDesc = {};
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = m_width;
		depthStencilDesc.Height = m_height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = m_depthStencilFormat;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = m_depthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optClear, IID_PPV_ARGS(&m_depthStencilBuffer)));

		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc;
		DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
		DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DSVDesc.Format = m_depthStencilFormat;
		DSVDesc.Texture2D.MipSlice = 0;
		m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &DSVDesc, GetDSVHeapCPUHandle());
	}

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	WaitForPreviousFrame();

	m_viewPort.TopLeftX = 0;
	m_viewPort.TopLeftY = 0;
	m_viewPort.Width    = static_cast<float>(m_width);
	m_viewPort.Height   = static_cast<float>(m_height);
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
}

bool D3D12App::InitWindowApplication() {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWinApp";

	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT rect = { (LONG)0, (LONG)0, (LONG)m_width, (LONG)m_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	m_hWnd = CreateWindow(L"MainWinApp", m_mainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hInst, 0);
	if (!m_hWnd) {
		MessageBox(0, L"CreateMainWinApp Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return true;
}

void D3D12App::WaitForPreviousFrame() {
	const UINT64 fence = m_currFenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_currFenceValue++;

	if (m_fence->GetCompletedValue() < fence) {
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

bool D3D12App::InitD3D12() {
#if defined(_DEBUG) 
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		}
	}
#endif
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));

	if (m_useWarpDevice) {
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
	}
	else {
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
	}

	m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CBVSRVUAVDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	BuildFence();
	BuildCommandObjects();
	BuildSwapChain();
	BuildAllDescriptorHeaps();

	WaitForPreviousFrame();

	return true;
}

bool D3D12App::InitD3D11() {
	ComPtr<ID3D11Device> d3d11Device;
	ThrowIfFailed(D3D11On12CreateDevice(m_device.Get(), d3d11DeviceFlags, nullptr, 0, reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()), 1, 0, &d3d11Device, &m_d3d11DeviceContext, nullptr));
	ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));

	{
		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));

		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
		ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext));
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
	}

	return true;
}

void D3D12App::BuildFence() {
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

	m_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_fenceEvent == NULL) {
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void D3D12App::BuildCommandObjects() {
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	m_commandList->Close();
}

void D3D12App::BuildSwapChain() {
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FRAME;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = m_backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hWnd, &swapChainDesc, nullptr, nullptr, &swapChain));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_currBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	ThrowIfFailed(m_factory->MakeWindowAssociation(GetHWND(), DXGI_MWA_NO_ALT_ENTER));
}

void D3D12App::CalculateFrame() {
	static UINT  frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_timer.GetTotalTime() - timeElapsed) >= 1.0f) {
		float fps  = (float)frameCnt;
		float mspf = 1000.0f / fps;

		wstring fpsStr  = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = m_mainWndCaption +
			L"   FPS: "  + fpsStr +
			L"   MSPF: " + mspfStr;

		SetWindowText(m_hWnd, windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::GetRTVHeapCPUHandle() const   { return m_rtvHeap->GetCPUDescriptorHandleForHeapStart(); }
D3D12_GPU_DESCRIPTOR_HANDLE D3D12App::GetRTVHeapGPUHandle() const   { return m_rtvHeap->GetGPUDescriptorHandleForHeapStart(); }
D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::GetDSVHeapCPUHandle() const   { return m_dsvHeap->GetCPUDescriptorHandleForHeapStart(); }
D3D12_GPU_DESCRIPTOR_HANDLE D3D12App::GetDSVHeapGPUHandle() const   { return m_dsvHeap->GetGPUDescriptorHandleForHeapStart(); }
D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::GetCurrBackBufferView() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currBackBufferIndex, m_RTVDescriptorSize); }
ID3D12Resource* D3D12App::GetCurrBackBuffer() const                 { return m_backBuffers[m_currBackBufferIndex].Get(); }
HINSTANCE D3D12App::GetHINST() const                                { return m_hInst; }
HWND D3D12App::GetHWND() const                                      { return m_hWnd; }
float D3D12App::GetAspectRatio() const                              { return static_cast<float>(m_width) / m_height; }
D3D12App* D3D12App::GetThis()                                       { return m_app; }

D3D12App* D3D12App::m_app = nullptr;