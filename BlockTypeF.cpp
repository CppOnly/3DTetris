#include "stdafx.h"
#include "BlockTypeF.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

BlockTypeF::BlockTypeF(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:Block(_device, _descHeap)
{
	ResetRelativeData();
}

BlockTypeF::~BlockTypeF()
{
	m_pGeometry.reset();
	m_pBorder.reset();
}



void BlockTypeF::OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess)
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

void BlockTypeF::ResetRelativeData()
{
	m_blockRelativePositions.resize(4);
	m_blockRelativePositions[0] = XMFLOAT3( 0.5f, -0.5f, -0.5f);
	m_blockRelativePositions[1] = XMFLOAT3( 0.5f, -0.5f,  0.5f);
	m_blockRelativePositions[2] = XMFLOAT3(-0.5f, -0.5f,  0.5f);
	m_blockRelativePositions[3] = XMFLOAT3(-0.5f, -0.5f,  1.5f);
}



void BlockTypeF::BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	m_pGeometry = std::make_unique<MeshGeometry>();
	m_pBorder = std::make_unique<MeshGeometry>();

	array<Vertex, 20> vertices = {
		Vertex({ XMFLOAT3( 0.0f,  0.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f,  0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, -1.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f,  0.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f,  0.0f,  2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f,  2.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) })
	};

	array<UINT, 56> indices = {
		16, 18, 14,  8, 14,  9, 12,  1,  0,  1,  2,  3,  6,  7, 
		 4,  5, 13,  5, 15, 11, 15, 10, 17, 19, 12,  0, 13,  4, 
		 0,  2,  4,  6,  2,  3,  6,  7,  3,  9,  7, 11,  9,  8, 
		11, 10,  8, 18, 10, 19, 18, 16, 19, 17, 16, 12, 17, 13
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



void BlockTypeF::BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	array<Vertex, 20> vertices = {
		Vertex({ XMFLOAT3( 0.0f,  0.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f,  0.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f,  0.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f,  0.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, -1.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f,  0.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f,  0.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  2.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  2.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f,  0.0f,  2.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3( 0.0f, -1.0f,  2.0f), XMFLOAT4(Colors::Black) })
	};

	array<UINT, 48> indices = {
		16, 18, 16, 17, 18, 19, 17, 19, 16, 12, 18,  2,
		 9,  3, 14,  9, 12,  1, 14, 15, 12, 13,  2,  6, 
		 3,  7,  2,  3,  8, 10,  0,  4,  1,  5,  9, 11, 
		17, 13, 19,  6, 11,  7,  6,  7, 13,  5, 15, 11
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

unique_ptr<MeshGeometry> BlockTypeF::m_pGeometry = nullptr;
unique_ptr<MeshGeometry> BlockTypeF::m_pBorder = nullptr;