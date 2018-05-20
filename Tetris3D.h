#pragma once
#include "stdafx.h"
#include "D3D12App.h"

#include "Grid.h"
#include "Block.h"
#include "AudioManager.h"
#include "BlockFactory.h"
#include "FrameResource.h"

class Tetris3D : public D3D12App
{
public:
	explicit Tetris3D(HINSTANCE _hInstance);
	virtual ~Tetris3D();

	Tetris3D() = delete;
	Tetris3D(const Tetris3D& rhs) = delete;
	Tetris3D& operator=(const Tetris3D& rhs) = delete;

public:
	virtual bool Initialize() override;

private:
	virtual void BuildAllDescriptorHeaps() override;
	virtual void OnResize() override;
	virtual void Update(GameTimer& _gt) override;
	virtual void Draw() override;

	virtual void OnMouseDown (WPARAM _btnState, int _x, int _y) override;
	virtual void OnMouseUp   (WPARAM _btnState, int _x, int _y) override;
	virtual void OnMouseMove (WPARAM _btnState, int _x, int _y) override;
	virtual void OnKeyPressed(WPARAM _key) override;

	void UpdateSceneConstantBuffer();

	void BuildCompliedShaders();
	void BuildRootSignature();
	void BuildPipelineStateObject();
	void BuildFrameResource();
	
	// Game Data Method
	void BuildGameData();
	void NewGame();
	void GameOver();
	void GameClear();
	void SetCurrAndNextBlock();
	void SetNextBlockForPreview();
	void MoveDownCurrBlock();
	float CalcBlockFallingTimeByLevel(UINT _level);

	// Multi-Threading Method
	void BuildThread();
	void BeginRendering();
	void EndRendering();
	void WorkerThread(int _index);
	void SetCommonPipelineState(ID3D12GraphicsCommandList* _cmdList);
	void SetCommonPipelineState_UI(ID3D12GraphicsCommandList* _cmdList);

	// UI Rendering Method
	void BuildConstantBuffer_UI();
	void BuildTextData();
	void OnRender_MainUI();
	void OnRender_PauseUI();
	void OnRender_GameOverUI();
	void OnRender_GameClearUI();

	// Network Method
	void SendScoreToServer();

private:
	std::unique_ptr<D3DUtil::UploadBuffer<SceneConstant>> m_constantBuffer_UI = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_VSByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_PSByteCode = nullptr;

	Microsoft::WRL::ComPtr<ID3DBlob> m_VSTessByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_PSTessByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_HSTessByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> m_DSTessByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineStateTess = nullptr;

	Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat = nullptr;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush = nullptr;

	DirectX::XMFLOAT4X4 m_projMat = Math::Identity4x4();
	DirectX::XMFLOAT3 m_eyePos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_eyePos1 = { 0.0f, 0.0f, 0.0f };
	WPARAM m_lastKeyPressed = 0;
	POINT m_lastMousePos;
	float m_theta = 1.5f * DirectX::XM_PI;
	float m_phi = DirectX::XM_PIDIV4;
	float m_radius = 20.0f;

	std::unique_ptr<Grid>  m_pGrid = nullptr;
	std::shared_ptr<Block> m_pCurrBlock = nullptr;
	std::shared_ptr<Block> m_pNextBlock = nullptr;

	std::shared_ptr<FrameResource> m_pFrameResources[FRAME] = { nullptr };
	std::shared_ptr<FrameResource> m_pCurrFrameResource = nullptr;
	UINT m_currFrameResourceIndex = 0;

	float m_blockFallingTime = 0.0f;
	float m_timeOfLastFall = 0.0f;
	UINT m_level = 0;
	UINT m_score = 0;
	UINT m_highestScore = 0;
	bool m_isGameOver = false;
	bool m_isGameClear = false;
	bool m_isGamePaused = false;

	struct ThreadParameter
	{
		UINT ThreadIndex;
	};
	ThreadParameter m_threadParameters[THREADNUM];
	HANDLE m_threadBeginRenderingHandles[THREADNUM] = { NULL };
	HANDLE m_threadEndRenderingHandles[THREADNUM] = { NULL };
	HANDLE m_threadHandles[THREADNUM] = { NULL };

	UINT64 m_fenceValue = 0;

	std::unique_ptr<AudioManager> m_audioManger = nullptr;

private:
	static Tetris3D* m_app;
};