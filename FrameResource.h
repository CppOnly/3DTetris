#pragma once
#include "stdafx.h"
#include "D3D12Helper.h"

class FrameResource final {
public:
	FrameResource(ID3D12Device* _device, ID3D12PipelineState* _pso, ID3D12DescriptorHeap* _descHeap, UINT _index);
	~FrameResource() = default;

	void Initialize();
	void WriteConstantBuffer(SceneConstant _constant);

public:
	ID3D12CommandList* m_batchSubmits[THREADNUM + 2] = { nullptr };

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_commandAllocators[2] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandLists[2] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_objectCommandAllocators[THREADNUM] = { nullptr };
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_objectCommandLists[THREADNUM] = { nullptr };

	UINT64 m_fenceValue = 0;

private:
	std::unique_ptr<D3DUtil::UploadBuffer<SceneConstant>> m_constantBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;

private:
	FrameResource() = delete;
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
};