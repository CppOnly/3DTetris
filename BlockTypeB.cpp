#include "stdafx.h"
#include "BlockTypeB.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

BlockTypeB::BlockTypeB(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:Block(_device, _descHeap)
{
	ResetRelativeData();
}

BlockTypeB::~BlockTypeB()
{
	m_pGeometry.reset();
	m_pBorder.reset();
}



void BlockTypeB::OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess)
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

void BlockTypeB::ResetRelativeData()
{
	m_blockRelativePositions.resize(4);
	m_blockRelativePositions[0] = XMFLOAT3( 0.5f, 0.5f, 0.5f);
	m_blockRelativePositions[1] = XMFLOAT3(-0.5f, 0.5f, 0.5f);
	m_blockRelativePositions[2] = XMFLOAT3( 1.5f, 0.5f, 0.5f);
	m_blockRelativePositions[3] = XMFLOAT3( 0.5f, 1.5f, 0.5f);
}



void BlockTypeB::BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	m_pGeometry = std::make_unique<MeshGeometry>();
	m_pBorder = std::make_unique<MeshGeometry>();

	array<Vertex, 20> vertices = {
		Vertex({ XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 2.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 2.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 2.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 2.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, 2.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, 2.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) })
	};

	array<UINT, 68> indices = {
		 0,  1,  2,  3,  2,  3,  6,  7, 5,  4,  7, 6,  1,  8,  3,  9,  5,
		10,  1,  8, 10,  5, 11,  7,  3, 9,  7, 11, 8, 10,  9, 11, 12,  0,
		13,  2, 14,  4, 12,  0,  4, 14, 6, 15, 13, 2, 15,  6, 14, 12, 15,
		13, 16, 17,  0,  1, 17, 19,  1, 5, 19, 18, 5,  4, 18, 16,  4,  0
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



void BlockTypeB::BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	array<Vertex, 20> vertices = {
		Vertex({ XMFLOAT3( 0.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 2.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 2.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 2.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 2.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, 2.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, 2.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, 2.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, 2.0f, 1.0f), XMFLOAT4(Colors::Black) })
	};

	array<UINT, 48> indices = {
		12,  8, 14, 10,  8, 10, 12, 14, 13,  9, 15, 11,
		11,  9, 15, 13, 12, 13, 14, 15,  8,  9, 10, 11,
		16,  2, 17,  3, 18,  6, 19,  7, 18, 19, 16, 17,
		18, 16, 19, 17,  1,  5,  0,  4,  2,  6,  3,  7
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

unique_ptr<MeshGeometry> BlockTypeB::m_pGeometry = nullptr;
unique_ptr<MeshGeometry> BlockTypeB::m_pBorder = nullptr;