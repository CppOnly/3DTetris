#include "stdafx.h"
#include "BlockTypeA.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

BlockTypeA::BlockTypeA(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:Block(_device, _descHeap)
{
	ResetRelativeData();
}

BlockTypeA::~BlockTypeA()
{
	m_pGeometry.reset();
	m_pBorder.reset();
}



void BlockTypeA::OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess)
{
	GameObject::UpdateConstantBuffer();
	GameObject::SetDescriptorTable(_device, _cmdList, _descHeap);

	_cmdList->SetPipelineState(_psoTess);
	_cmdList->IASetVertexBuffers(0, 1, &m_pGeometry->GetVertexBufferView());
	_cmdList->IASetIndexBuffer(&m_pGeometry->GetIndexBufferView());
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	_cmdList->DrawIndexedInstanced(m_pGeometry->IndexNum, 1, 0, 0, 0);

	_cmdList->SetPipelineState(_pso);
	_cmdList->IASetVertexBuffers(0, 1, &m_pBorder->GetVertexBufferView());
	_cmdList->IASetIndexBuffer(&m_pBorder->GetIndexBufferView());
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	_cmdList->DrawIndexedInstanced(m_pBorder->IndexNum, 1, 0, 0, 0);
}

void BlockTypeA::ResetRelativeData()
{
	m_blockRelativePositions.resize(1);
	m_blockRelativePositions[0] = XMFLOAT3(0.5f, 0.5f, 0.5f);
}



void BlockTypeA::BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	m_pGeometry = std::make_unique<MeshGeometry>();
	m_pBorder = std::make_unique<MeshGeometry>();

	array<Vertex, 8> vertices = {
		Vertex({ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) })
	};

	array<UINT, 24> indices = {
		0, 1, 2, 3, 5, 4, 7, 6,
		4, 5, 0, 1, 2, 3, 6, 7,
		1, 5, 3, 7, 4, 0, 6, 2
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

	m_pGeometry->VertexBuffer = D3DUtil::CreateDefaultBuffer(_device, _cmdList, vertices.data(), vbByteSize, m_pGeometry->VertexBufferUpload);
	m_pGeometry->IndexBuffer = D3DUtil::CreateDefaultBuffer(_device, _cmdList, indices.data(), ibByteSize, m_pGeometry->IndexBufferUpload);

	m_pGeometry->VertexByteStride = sizeof(Vertex);
	m_pGeometry->VBByteSize = vbByteSize;
	m_pGeometry->IBByteSize = ibByteSize;
	m_pGeometry->IndexNum = (UINT)indices.size();

	BuildBorder(_device, _cmdList);
}



void BlockTypeA::BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	array<Vertex, 8> vertices = {
		Vertex({ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) })
	};

	array<UINT, 24> indices = {
		0, 1, 0, 2, 1, 3, 2, 3,
		4, 5, 4, 6, 5, 7, 6, 7,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

	m_pBorder->VertexBuffer = D3DUtil::CreateDefaultBuffer(_device, _cmdList, vertices.data(), vbByteSize, m_pBorder->VertexBufferUpload);
	m_pBorder->IndexBuffer = D3DUtil::CreateDefaultBuffer(_device, _cmdList, indices.data(), ibByteSize, m_pBorder->IndexBufferUpload);

	m_pBorder->VertexByteStride = sizeof(Vertex);
	m_pBorder->VBByteSize = vbByteSize;
	m_pBorder->IBByteSize = ibByteSize;
	m_pBorder->IndexNum = (UINT)indices.size();
}

unique_ptr<MeshGeometry> BlockTypeA::m_pGeometry = nullptr;
unique_ptr<MeshGeometry> BlockTypeA::m_pBorder = nullptr;