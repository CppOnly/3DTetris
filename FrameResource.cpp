#include "stdafx.h"
#include "FrameResource.h"

using namespace std;

FrameResource::FrameResource(ID3D12Device* _device, ID3D12PipelineState* _pso, ID3D12DescriptorHeap* _descHeap, UINT _index)
	:m_pipelineState(_pso)
{
	for (UINT i = 0; i < 2; ++i) {
		ThrowIfFailed(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
		ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandLists[i])));
		ThrowIfFailed(m_commandLists[i]->Close());
	}

	for (UINT i = 0; i < THREADNUM; ++i) {
		ThrowIfFailed(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_objectCommandAllocators[i])));
		ThrowIfFailed(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_objectCommandAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_objectCommandLists[i])));
		ThrowIfFailed(m_objectCommandLists[i]->Close());
	}

	m_constantBuffer = std::make_unique<D3DUtil::UploadBuffer<SceneConstant>>(_device, 1, true);
	D3D12_GPU_VIRTUAL_ADDRESS address = m_constantBuffer->GetUploadBuffer()->GetGPUVirtualAddress();
	UINT size = D3DUtil::CalcConstantBufferByteSize(sizeof(SceneConstant));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = address;
	cbvDesc.SizeInBytes = size;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_descHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(_index, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	_device->CreateConstantBufferView(&cbvDesc, handle);

	const UINT batchSize = THREADNUM + 2;
	m_batchSubmits[0] = m_commandLists[0].Get();
	memcpy(m_batchSubmits + 1, m_objectCommandLists, _countof(m_objectCommandLists) * sizeof(ID3D12CommandList*));
	m_batchSubmits[batchSize - 1] = m_commandLists[1].Get();
}

FrameResource::~FrameResource()
{
	m_constantBuffer.reset();
}

void FrameResource::Initialize()
{
	for (UINT i = 0; i < 2; ++i) {
		ThrowIfFailed(m_commandAllocators[i]->Reset());
		ThrowIfFailed(m_commandLists[i]->Reset(m_commandAllocators[i].Get(), m_pipelineState.Get()));
	}

	for (UINT i = 0; i < THREADNUM; ++i) {
		ThrowIfFailed(m_objectCommandAllocators[i]->Reset());
		ThrowIfFailed(m_objectCommandLists[i]->Reset(m_objectCommandAllocators[i].Get(), m_pipelineState.Get()));
	}
}

void FrameResource::WriteConstantBuffer(SceneConstant _constant)
{
	m_constantBuffer->CopyData(0, _constant);
}