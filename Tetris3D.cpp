#include "stdafx.h"
#include "Tetris3D.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

Tetris3D::Tetris3D(HINSTANCE _hInstance)
	:D3D12App(_hInstance)
{
	assert(m_app == nullptr);
	m_app = this;
}

Tetris3D::~Tetris3D()
{
	m_pGrid.reset();
	m_pCurrBlock.reset();
	m_pNextBlock.reset();
	m_audioManger.reset();

	for (auto &a : m_pFrameResources) {
		a.reset();
	}
	m_pCurrFrameResource.reset();

	for (UINT i = 0; i < THREADNUM; ++i) {
		CloseHandle(m_threadBeginRenderingHandles[i]);
		CloseHandle(m_threadEndRenderingHandles[i]);
		CloseHandle(m_threadHandles[i]);
	}

	BlockFactory::DestoryBlockFactory();

	m_app = nullptr;
}



bool Tetris3D::Initialize()
{
	if (!D3D12App::Initialize()) {
		return false;
	}

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	m_audioManger = make_unique<AudioManager>();

	BuildCompliedShaders();
	BuildRootSignature();
	BuildPipelineStateObject();
	BuildFrameResource();
	BuildGameData();
	BuildThread();
	BuildConstantBuffer_UI();
	BuildTextData();

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	WaitForPreviousFrame();
	m_fence->Signal(0);

	return true;
}



void Tetris3D::BuildAllDescriptorHeaps()
{
	D3D12App::BuildAllDescriptorHeaps();

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 400;
	cbvHeapDesc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
}

void Tetris3D::OnResize()
{
	D3D12App::OnResize();

	XMMATRIX projMat = XMMatrixPerspectiveFovLH(0.25f * XM_PI, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_projMat, projMat);

	// 이 Game은 창에 대한 조작을 할시 Rendering이 멈추도록 되어 있다. 따라서 m_fence가 갱신되지 않아 멈추므로 OnResize시 수동으로 m_fence를 갱신해줘야 한다.
	m_fence->Signal(m_fenceValue);
}

void Tetris3D::Update(GameTimer& _gt)
{
	switch (m_lastKeyPressed)
	{
	case VK_RETURN:
		if (m_isGameOver || m_isGameClear) {
			// 새 Game을 시작하면 Win or Lose Audio를 끈다.
			m_audioManger->StopWinAudio();
			m_audioManger->StopLoseAudio();
			NewGame();
		}
		break;
	case VK_ESCAPE:
		::PostQuitMessage(0);
		break;
	}

	if (m_isGameOver || m_isGameClear) {
		return;
	}

	switch (m_lastKeyPressed)
	{
	case 'P':
		if (m_isGamePaused == true) {
			m_audioManger->PlayMainAudio();
		}
		else {
			m_audioManger->StopMainAudio();
		}
		m_isGamePaused = !m_isGamePaused;
		m_isGamePaused ? _gt.Stop() : _gt.Start();
		m_lastKeyPressed = 0;
		break;
	}

	if (m_isGamePaused) {
		return;
	}

	m_timeOfLastFall += _gt.GetDeltaTime();

	if (m_lastKeyPressed != 0) {

		if (m_eyePos.y <= 5 && m_eyePos.y >= 0 && m_eyePos.x < 0) {
			switch (m_lastKeyPressed)
			{
			case 'A':
				m_pCurrBlock->TryTranslate(0, 1, 0);
				break;
			case 'D':
				m_pCurrBlock->TryTranslate(0, -1, 0);
				break;
			case 'W':
				m_pCurrBlock->TryTranslate(1, 0, 0);
				break;
			case 'S':
				m_pCurrBlock->TryTranslate(-1, 0, 0);
				break;
			case 'Q':
				m_pCurrBlock->TryRotate(0, -XM_PIDIV2, 0);
				break;
			case 'E':
				m_pCurrBlock->TryRotate(0, 0, XM_PIDIV2);
				break;
			}
		}
		else if (m_eyePos.y < 0) {
			switch (m_lastKeyPressed)
			{
			case 'A':
				m_pCurrBlock->TryTranslate(-1, 0, 0);
				break;
			case 'D':
				m_pCurrBlock->TryTranslate(1, 0, 0);
				break;
			case 'W':
				m_pCurrBlock->TryTranslate(0, 1, 0);
				break;
			case 'S':
				m_pCurrBlock->TryTranslate(0, -1, 0);
				break;
			case 'Q':
				m_pCurrBlock->TryRotate(XM_PIDIV2, 0, 0);
				break;
			case 'E':
				m_pCurrBlock->TryRotate(0, 0, XM_PIDIV2);
				break;
			}
		}
		else if (m_eyePos.y > 6) {
			switch (m_lastKeyPressed)
			{
			case 'A':
				m_pCurrBlock->TryTranslate(1, 0, 0);
				break;
			case 'D':
				m_pCurrBlock->TryTranslate(-1, 0, 0);
				break;
			case 'W':
				m_pCurrBlock->TryTranslate(0, -1, 0);
				break;
			case 'S':
				m_pCurrBlock->TryTranslate(0, 1, 0);
				break;
			case 'Q':
				m_pCurrBlock->TryRotate(-XM_PIDIV2, 0, 0);
				break;
			case 'E':
				m_pCurrBlock->TryRotate(0, 0, XM_PIDIV2);
				break;
			}
		}
		else {
			switch (m_lastKeyPressed)
			{
			case 'A':
				m_pCurrBlock->TryTranslate(0, -1, 0);
				break;
			case 'D':
				m_pCurrBlock->TryTranslate(0, 1, 0);
				break;
			case 'W':
				m_pCurrBlock->TryTranslate(-1, 0, 0);
				break;
			case 'S':
				m_pCurrBlock->TryTranslate(1, 0, 0);
				break;
			case 'Q':
				m_pCurrBlock->TryRotate(0, XM_PIDIV2, 0);
				break;
			case 'E':
				m_pCurrBlock->TryRotate(0, 0, XM_PIDIV2);
				break;
			}
		}
		switch (m_lastKeyPressed)
		{
		case VK_SPACE:
			MoveDownCurrBlock();
			break;

		case 'T':
			m_score += 50;
			break;
		}
		if (m_lastKeyPressed != VK_SPACE) {
			m_lastKeyPressed = 0;
		}
	}
	else if (m_blockFallingTime <= m_timeOfLastFall) {
		MoveDownCurrBlock();
		m_timeOfLastFall = 0.0f;
	}

	if (m_score >= 100 * (m_level + 1) && m_level < FINAL_LEVEL) {
		++m_level;
		if (m_level == FINAL_LEVEL) {
			GameClear();
		}
		else {
			m_blockFallingTime = CalcBlockFallingTimeByLevel(m_level);
		}
	}

	m_pNextBlock->Rotate(0.001f, 0.0f, 0.0f);

	UpdateSceneConstantBuffer();
}

void Tetris3D::Draw()
{
	BeginRendering();

	for (UINT i = 0; i < THREADNUM; ++i) {
		SetEvent(m_threadBeginRenderingHandles[i]);
	}

	m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_currBackBufferIndex].GetAddressOf(), 1);

	if (!m_isGameOver && !m_isGamePaused && !m_isGameClear) {
		OnRender_MainUI();
	}
	else if (m_isGamePaused) {

		OnRender_MainUI();
		OnRender_PauseUI();
	}
	else if (m_isGameClear) {
		OnRender_MainUI();
		OnRender_GameClearUI();
	}
	else if (m_isGameOver) {
		OnRender_GameOverUI();
	}

	WaitForMultipleObjects(THREADNUM, m_threadEndRenderingHandles, TRUE, INFINITE);

	m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_currBackBufferIndex].GetAddressOf(), 1);

	EndRendering();

	m_commandQueue->ExecuteCommandLists(_countof(m_pCurrFrameResource->m_batchSubmits), m_pCurrFrameResource->m_batchSubmits);

	m_d3d11DeviceContext->Flush();

	// 후면 버퍼 출력 및 후면 버퍼 색인 갱신
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_currBackBufferIndex = (m_currBackBufferIndex + 1) % FRAME;

	// 현재 FrameResource의 Value를 m_fenceValue로 설정.
	m_pCurrFrameResource->m_fenceValue = m_fenceValue;
	// GPU의 Draw Call이 끝나면 m_fence의 값을 m_fenceValue로 설정.
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValue));
	// m_fenceValue를 1늘린다.
	m_fenceValue++;
}



void Tetris3D::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	SetCapture(m_hWnd);
}

void Tetris3D::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Tetris3D::OnMouseMove(WPARAM _btnState, int _x, int _y)
{
	if ((_btnState & MK_LBUTTON) != 0) {
		float dx = XMConvertToRadians(0.1f * static_cast<float>(_x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.1f * static_cast<float>(_y - m_lastMousePos.y));

		m_theta += dx;
		m_phi += dy;
		m_phi = Math::Clamp(m_phi, 0.0f, XM_PI);
	}
	else if ((_btnState & MK_RBUTTON) != 0) {
		float dx = 0.01f * static_cast<float>(_x - m_lastMousePos.x);
		float dy = 0.01f * static_cast<float>(_y - m_lastMousePos.y);

		m_radius += dx - dy;
		m_radius = Math::Clamp(m_radius, 10.0f, 20.0f);
	}

	m_lastMousePos.x = _x;
	m_lastMousePos.y = _y;
}

void Tetris3D::OnKeyPressed(WPARAM _key)
{
	m_lastKeyPressed = _key;
}



void Tetris3D::UpdateSceneConstantBuffer()
{
	const UINT64 lastCompletedFence = m_fence->GetCompletedValue();

	// FrameResource 교체.
	m_currFrameResourceIndex = (m_currFrameResourceIndex + 1) % FRAME;
	m_pCurrFrameResource = m_pFrameResources[m_currFrameResourceIndex];

	// 교체한 FrameResource가 사용중이라면(아직 Rendering이 덜 끝났다면) 어쩔 수 없이 CPU/GPU를 동기화한다.
	// 확인 방법 : Draw 함수가 실행되면 FrameResource의 value값은 m_fenceValue가 된다. 중요한 점은 m_fenceValue는 계속 늘어난다.
	// 하지만 Pipeline의 작업이 끝나지 않았다면 m_fence의 값은 m_fenceValue로 갱신이 안된다. 즉, 아래의 조건이 적용된다.
	if (m_pCurrFrameResource->m_fenceValue > lastCompletedFence) {
		HANDLE eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (eventHandle == NULL) {
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_pCurrFrameResource->m_fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	// 구면 좌표계를 데카르크 좌표계로 변경한다
	m_eyePos.x = m_radius * sinf(m_phi) * cosf(m_theta);
	m_eyePos.y = m_radius * sinf(m_phi) * sinf(m_theta);
	m_eyePos.z = m_radius * cosf(m_phi);

    // 시야 행렬을 만든다
	XMVECTOR pos = XMVectorSet(m_eyePos.x, m_eyePos.y, m_eyePos.z, 1.0f);
	XMVECTOR target = XMVectorSet(2.5f, 2.5f, 6.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMMATRIX viewMat = XMMatrixLookAtLH(pos, target, up);

	// 상수 자원값을 업데이트하고 Buffer를 변경한다
	XMMATRIX projMat = XMLoadFloat4x4(&m_projMat);
	XMMATRIX viewProjMat = viewMat * projMat;

	SceneConstant constant;
	XMStoreFloat4x4(&constant.ViewProjMat, XMMatrixTranspose(viewProjMat));
	constant.EyePosW = m_eyePos;

	m_pCurrFrameResource->WriteConstantBuffer(constant);
}



void Tetris3D::BuildCompliedShaders()
{
	m_VSByteCode = D3DUtil::CompileShader(L"Shaders\\Shader.hlsl", nullptr, "VS", "vs_5_0");
	m_PSByteCode = D3DUtil::CompileShader(L"Shaders\\Shader.hlsl", nullptr, "PS", "ps_5_0");

	m_VSTessByteCode = D3DUtil::CompileShader(L"Shaders\\ShaderTess.hlsl", nullptr, "VS", "vs_5_0");
	m_HSTessByteCode = D3DUtil::CompileShader(L"Shaders\\ShaderTess.hlsl", nullptr, "HS", "hs_5_0");
	m_DSTessByteCode = D3DUtil::CompileShader(L"Shaders\\ShaderTess.hlsl", nullptr, "DS", "ds_5_0");
	m_PSTessByteCode = D3DUtil::CompileShader(L"Shaders\\ShaderTess.hlsl", nullptr, "PS", "ps_5_0");

	m_inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void Tetris3D::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE range[2];
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	slotRootParameter[0].InitAsDescriptorTable(1, &range[0]);
	slotRootParameter[1].InitAsDescriptorTable(1, &range[1]);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> singature = nullptr;
	ComPtr<ID3DBlob> error = nullptr;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, singature.GetAddressOf(), error.GetAddressOf()));
	ThrowIfFailed(m_device->CreateRootSignature(0, singature->GetBufferPointer(), singature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
}

void Tetris3D::BuildPipelineStateObject()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<BYTE*>(m_VSByteCode->GetBufferPointer()), m_VSByteCode->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<BYTE*>(m_PSByteCode->GetBufferPointer()), m_PSByteCode->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.DSVFormat = m_depthStencilFormat;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoTessDesc = {};

	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoTessDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoTessDesc.pRootSignature = m_rootSignature.Get();
	psoTessDesc.VS = { reinterpret_cast<BYTE*>(m_VSTessByteCode->GetBufferPointer()), m_VSTessByteCode->GetBufferSize() };
	psoTessDesc.HS = { reinterpret_cast<BYTE*>(m_HSTessByteCode->GetBufferPointer()), m_HSTessByteCode->GetBufferSize() };
	psoTessDesc.DS = { reinterpret_cast<BYTE*>(m_DSTessByteCode->GetBufferPointer()), m_DSTessByteCode->GetBufferSize() };
	psoTessDesc.PS = { reinterpret_cast<BYTE*>(m_PSTessByteCode->GetBufferPointer()), m_PSTessByteCode->GetBufferSize() };
	psoTessDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoTessDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoTessDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoTessDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoTessDesc.BlendState.RenderTarget->BlendEnable = true;
	psoTessDesc.BlendState.RenderTarget->LogicOpEnable = false;
	psoTessDesc.BlendState.RenderTarget->SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoTessDesc.BlendState.RenderTarget->DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoTessDesc.BlendState.RenderTarget->BlendOp = D3D12_BLEND_OP_ADD;
	psoTessDesc.BlendState.RenderTarget->SrcBlendAlpha = D3D12_BLEND_ONE;
	psoTessDesc.BlendState.RenderTarget->DestBlendAlpha = D3D12_BLEND_ZERO;
	psoTessDesc.BlendState.RenderTarget->BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoTessDesc.BlendState.RenderTarget->LogicOp = D3D12_LOGIC_OP_NOOP;
	psoTessDesc.BlendState.RenderTarget->RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoTessDesc.BlendState.AlphaToCoverageEnable = false;
	psoTessDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoTessDesc.SampleMask = UINT_MAX;
	psoTessDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	psoTessDesc.NumRenderTargets = 1;
	psoTessDesc.RTVFormats[0] = m_backBufferFormat;
	psoTessDesc.DSVFormat = m_depthStencilFormat;
	psoTessDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoTessDesc, IID_PPV_ARGS(&m_pipelineStateTess)));
}

void Tetris3D::BuildFrameResource()
{
	for (int i = 0; i < FRAME; ++i) {
		m_pFrameResources[i] = make_shared<FrameResource>(m_device.Get(), m_pipelineState.Get(), m_cbvHeap.Get(), i);
	}

	m_currFrameResourceIndex = 0;
	m_pCurrFrameResource = m_pFrameResources[m_currFrameResourceIndex];
}



void Tetris3D::BuildGameData()
{
	BlockFactory::BuildAllBlockTypeGeometry(m_device.Get(), m_commandList.Get(), m_cbvHeap.Get());

	m_pGrid = make_unique<Grid>(m_device.Get(), m_cbvHeap.Get());
	m_pGrid->BuildGeometry(m_device.Get(), m_commandList.Get());

	NewGame();
}

void Tetris3D::NewGame()
{
	m_audioManger->PlayMainAudio();

	m_level = 0;
	m_score = 0;
	m_timeOfLastFall = 0.0f;
	m_blockFallingTime = CalcBlockFallingTimeByLevel(0);
	m_lastKeyPressed = 0;
	m_isGameOver = false;
	m_isGameClear = false;
	m_pGrid->ResetStackedBlocksData();

	SetCurrAndNextBlock();
	m_timer.Reset();
}

void Tetris3D::GameOver()
{
	m_audioManger->StopMainAudio();
	m_audioManger->PlayLoseAudio();

	SendScoreToServer();

	m_timer.Stop();
	m_isGameOver = true;
}

void Tetris3D::GameClear()
{
	m_audioManger->StopMainAudio();
	m_audioManger->PlayWinAudio();

	SendScoreToServer();

	m_timer.Stop();
	m_isGameClear = true;
}

void Tetris3D::SetCurrAndNextBlock()
{
	m_pCurrBlock = BlockFactory::SelectRandomBlock();
	m_pCurrBlock->SetPosition(2.0f, 2.0f, 1.0f);

	SetNextBlockForPreview();
}

void Tetris3D::SetNextBlockForPreview()
{
	m_pNextBlock = BlockFactory::SelectRandomBlock();
	m_pNextBlock->SetPosition(-2.5f, 1.3f, 1.0f);
	m_pNextBlock->SetScale(0.2f, 0.2f, 0.2f);
}

void Tetris3D::MoveDownCurrBlock()
{
	if (!m_pCurrBlock->TryTranslate(0.0f, 0.0f, 1.0f)) {
  		m_pCurrBlock->StackInGrid();
		m_audioManger->PlayDropAudio();
 		m_score += m_pGrid->CalcRemoveNum() * (m_level + 1) * 10;

		if (Grid::m_highestLayerOfStackedBlocks == 1) {
			GameOver();
		}

		m_pCurrBlock = m_pNextBlock;
		m_pCurrBlock->SetPosition(2.0f, 2.0f, 1.0f);
		m_pCurrBlock->SetRotation(0.0f, 0.0f, 0.0f);
		m_pCurrBlock->SetScale(1.0f, 1.0f, 1.0f);
		m_pCurrBlock->ResetRelativeData();
		SetNextBlockForPreview();

		m_lastKeyPressed = 0;
		m_timeOfLastFall = 0.0f;
	}
}

float Tetris3D::CalcBlockFallingTimeByLevel(UINT _level)
{
	assert(_level <= FINAL_LEVEL);

	return (FINAL_LEVEL - _level) * 0.1f;
}



void Tetris3D::BuildThread()
{
	struct threadwrapper
	{
		static UINT WINAPI thunk(LPVOID _lpParameter)
		{
			ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(_lpParameter);
			m_app->WorkerThread(parameter->ThreadIndex);
			return 0;
		}
	};

	for (UINT i = 0; i < THREADNUM; ++i) {
		m_threadBeginRenderingHandles[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_threadEndRenderingHandles[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_threadParameters[i].ThreadIndex = i;
		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, threadwrapper::thunk, reinterpret_cast<LPVOID>(&m_threadParameters[i]), 0, nullptr));

		assert(m_threadBeginRenderingHandles[i] != NULL);
		assert(m_threadEndRenderingHandles[i] != NULL);
		assert(m_threadHandles[i] != NULL);
	}
}

void Tetris3D::BeginRendering()
{
	m_pCurrFrameResource->Initialize();
	
	m_pCurrFrameResource->m_commandLists[0].Get()->RSSetViewports(1, &m_viewPort);
	m_pCurrFrameResource->m_commandLists[0].Get()->RSSetScissorRects(1, &m_scissorRect);
	m_pCurrFrameResource->m_commandLists[0].Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_pCurrFrameResource->m_commandLists[0].Get()->ClearRenderTargetView(GetCurrBackBufferView(), Colors::Black, 0, nullptr);
	m_pCurrFrameResource->m_commandLists[0].Get()->ClearDepthStencilView(GetDSVHeapCPUHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_pCurrFrameResource->m_commandLists[0].Get()->OMSetRenderTargets(1, &GetCurrBackBufferView(), true, &GetDSVHeapCPUHandle());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_pCurrFrameResource->m_commandLists[0].Get()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	m_pCurrFrameResource->m_commandLists[0].Get()->SetGraphicsRootSignature(m_rootSignature.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(m_currFrameResourceIndex, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	m_pCurrFrameResource->m_commandLists[0].Get()->SetGraphicsRootDescriptorTable(1, handle);

	ThrowIfFailed(m_pCurrFrameResource->m_commandLists[0]->Close());
}

void Tetris3D::EndRendering()
{
	m_pCurrFrameResource->m_commandLists[1]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GetCurrBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_pCurrFrameResource->m_commandLists[1]->Close());
}

void Tetris3D::WorkerThread(int _index)
{
	assert(_index >= 0 && _index < THREADNUM);

	while (_index >= 0 && _index < THREADNUM) {
		WaitForSingleObject(m_threadBeginRenderingHandles[_index], INFINITE);

		ID3D12GraphicsCommandList* pCommandList = m_pCurrFrameResource->m_objectCommandLists[_index].Get();
		
		if (_index == 0) {
			SetCommonPipelineState(pCommandList);
			m_pGrid->OnRender(m_device.Get(), pCommandList, m_cbvHeap.Get(), m_pipelineState.Get(), m_pipelineStateTess.Get());
			pCommandList->Close();
		}
		else if (_index == 1) {
			SetCommonPipelineState(pCommandList);
			m_pCurrBlock->OnRender(m_device.Get(), pCommandList, m_cbvHeap.Get(), m_pipelineState.Get(), m_pipelineStateTess.Get());
			pCommandList->Close();
		}
		else if (_index == 2) {
			SetCommonPipelineState_UI(pCommandList);
			m_pNextBlock->OnRender(m_device.Get(), pCommandList, m_cbvHeap.Get(), m_pipelineState.Get(), m_pipelineStateTess.Get());
			pCommandList->Close();
		}

		SetEvent(m_threadEndRenderingHandles[_index]);
	}
}

void Tetris3D::SetCommonPipelineState(ID3D12GraphicsCommandList* _cmdList)
{
	_cmdList->RSSetViewports(1, &m_viewPort);
	_cmdList->OMSetRenderTargets(1, &GetCurrBackBufferView(), true, &GetDSVHeapCPUHandle());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	_cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
}

void Tetris3D::SetCommonPipelineState_UI(ID3D12GraphicsCommandList* _cmdList)
{
	_cmdList->RSSetViewports(1, &m_viewPort);
	_cmdList->OMSetRenderTargets(1, &GetCurrBackBufferView(), true, &GetDSVHeapCPUHandle());

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	_cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(399, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	_cmdList->SetGraphicsRootDescriptorTable(1, handle);
}



void Tetris3D::BuildConstantBuffer_UI()
{
	m_constantBuffer_UI = std::make_unique<D3DUtil::UploadBuffer<SceneConstant>>(m_device.Get(), 1, true);

	D3D12_GPU_VIRTUAL_ADDRESS address = m_constantBuffer_UI->GetUploadBuffer()->GetGPUVirtualAddress();
	UINT size = D3DUtil::CalcConstantBufferByteSize(sizeof(SceneConstant));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = address;
	cbvDesc.SizeInBytes = size;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(399, m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	m_device->CreateConstantBufferView(&cbvDesc, handle);

	XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f);
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX viewMat = XMMatrixLookAtLH(pos, target, up);

	// 상수 자원값을 업데이트하고 Buffer를 변경한다
	XMMATRIX projMat = XMLoadFloat4x4(&m_projMat);
	XMMATRIX viewProjMat = viewMat * projMat;

	SceneConstant constant;
	XMStoreFloat4x4(&constant.ViewProjMat, XMMatrixTranspose(viewProjMat));
	m_constantBuffer_UI->CopyData(0, constant);
}

void Tetris3D::BuildTextData()
{
	ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_textBrush));
	ThrowIfFailed(m_dWriteFactory->CreateTextFormat(L"Cambria", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30, L"en-us", &m_textFormat));
	ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
}

void Tetris3D::OnRender_MainUI()
{
	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_currBackBufferIndex]->GetSize();

	D2D1_RECT_F nextBlocktextRect = D2D1::RectF(0.0f, 0.0f, rtSize.width / 4.0f, rtSize.height / 4.0f);
	static const WCHAR nextBlockText[] = L"Next Block";

	D2D1_RECT_F levelTextRect = D2D1::RectF(0.0f, 0.0f, rtSize.width * 1.7f, rtSize.height * 0.45f);
	WCHAR levelText[50] = L"Level : ";
	const WCHAR levelNumText[] = { m_level + L'0', L'\0' };
	wcscat_s(levelText, levelNumText);

	D2D1_RECT_F scoreTextRect = D2D1::RectF(0.0f, 0.0f, rtSize.width * 1.7f, rtSize.height * 1.45f);
	WCHAR scoreText[50] = L"Score : ";
	WCHAR scoreNumText[5];
	_itow(m_score, scoreNumText, 10);
	wcscat_s(scoreText, scoreNumText);

	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_currBackBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	m_d2dDeviceContext->DrawText(nextBlockText, wcslen(nextBlockText), m_textFormat.Get(), &nextBlocktextRect, m_textBrush.Get());
	m_d2dDeviceContext->DrawText(levelText, wcslen(levelText), m_textFormat.Get(), &levelTextRect, m_textBrush.Get());
	m_d2dDeviceContext->DrawText(scoreText, wcslen(scoreText), m_textFormat.Get(), &scoreTextRect, m_textBrush.Get());
	ThrowIfFailed(m_d2dDeviceContext->EndDraw());
}

void Tetris3D::OnRender_PauseUI()
{
	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_currBackBufferIndex]->GetSize();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
	static const WCHAR text[] = L"------Pause------";

	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_currBackBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	m_d2dDeviceContext->DrawText(text, wcslen(text), m_textFormat.Get(), &textRect, m_textBrush.Get());
	ThrowIfFailed(m_d2dDeviceContext->EndDraw());
}

void Tetris3D::OnRender_GameOverUI()
{
	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_currBackBufferIndex]->GetSize();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
	static const WCHAR text[] = L"------Game Over------";

	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_currBackBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	m_d2dDeviceContext->DrawText(text, wcslen(text), m_textFormat.Get(), &textRect, m_textBrush.Get());
	ThrowIfFailed(m_d2dDeviceContext->EndDraw());
}

void Tetris3D::OnRender_GameClearUI()
{
	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_currBackBufferIndex]->GetSize();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
	static const WCHAR text[] = L"------Game Clear------";

	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_currBackBufferIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	m_d2dDeviceContext->DrawText(text, wcslen(text), m_textFormat.Get(), &textRect, m_textBrush.Get());
	ThrowIfFailed(m_d2dDeviceContext->EndDraw());
}



void Tetris3D::SendScoreToServer()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		return;
	}

	SOCKET serverSocket;
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET) {
		return;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.S_un.S_addr = inet_addr(IP);

	if (connect(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(serverSocket);
		return;
	}

	send(serverSocket, (char*)&m_score, sizeof(UINT), 0);

	closesocket(serverSocket);
	WSACleanup();
}

Tetris3D* Tetris3D::m_app = nullptr;