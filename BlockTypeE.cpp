#include "stdafx.h"
#include "BlockTypeE.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

BlockTypeE::BlockTypeE(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:Block(_device, _descHeap)
{
	ResetRelativeData();
}

BlockTypeE::~BlockTypeE()
{
	m_pGeometry.reset();
	m_pBorder.reset();
}



void BlockTypeE::OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess)
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

void BlockTypeE::ResetRelativeData()
{
	m_blockRelativePositions.resize(4);
	m_blockRelativePositions[0] = XMFLOAT3( 0.5f, -0.5f, -0.5f);
	m_blockRelativePositions[1] = XMFLOAT3( 0.5f, -0.5f,  0.5f);
	m_blockRelativePositions[2] = XMFLOAT3(-0.5f, -0.5f,  0.5f);
	m_blockRelativePositions[3] = XMFLOAT3(-0.5f, -0.5f, -0.5f);
}



void BlockTypeE::BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	m_pGeometry = std::make_unique<MeshGeometry>();
	m_pBorder = std::make_unique<MeshGeometry>();

	array<Vertex, 18> vertices = {
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
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f) })
	};

	array<UINT, 24> indices = {
		12, 9, 16,  3, 17,  7, 13, 11, 16,  3, 17,  7, 
		 3, 9, 7 , 11,  9, 12, 11, 13, 12, 16, 13, 17
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



void BlockTypeE::BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	array<Vertex, 18> vertices = {
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
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f,  0.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Black) })
	};

	array<UINT, 40> indices = {
		12,  9, 16,  3, 12, 16,  9,  3, 17, 13, 
		 7, 11, 17,  7, 13, 11,  2,  8, 14,  1, 
		 6, 10,  5, 15,  2,  6,  3,  7,  1,  5, 
		 9, 11,  8, 10, 12, 13, 14, 15, 16, 17
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

unique_ptr<MeshGeometry> BlockTypeE::m_pGeometry = nullptr;
unique_ptr<MeshGeometry> BlockTypeE::m_pBorder = nullptr;