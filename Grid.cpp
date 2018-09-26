#include "stdafx.h"
#include "Grid.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

StackedBlock::StackedBlock(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:GameObject(_device, _descHeap) 
{}

StackedBlock::~StackedBlock()
{
	m_pGeometry.reset();
	m_pBorder.reset();
}



void StackedBlock::OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess)
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



void StackedBlock::BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	m_pGeometry = std::make_unique<MeshGeometry>();
	m_pBorder = std::make_unique<MeshGeometry>();

	array<Vertex, 8> vertices = {
		Vertex({ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(Colors::SlateGray) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT4(Colors::SlateGray) })
	};

	array<UINT, 24> indices = {
		1, 2, 0, 3, 5, 6, 4, 7,
		5, 1, 4, 0, 2, 6, 3, 7,
		5, 6, 1, 2, 0, 3, 4, 7
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



void StackedBlock::BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	array<Vertex, 8> vertices = {
		Vertex({ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT4(Colors::Black) })
	};

	array<UINT, 24> indices = {
		0, 1, 0, 3, 1, 2, 2, 3,
		4, 5, 4, 7, 5, 6, 6, 7,
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

unique_ptr<MeshGeometry> StackedBlock::m_pGeometry = nullptr;
unique_ptr<MeshGeometry> StackedBlock::m_pBorder = nullptr;





Grid::Grid(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:GameObject(_device, _descHeap)
{
	// 300������ StackedBlock�� �̸� �����д�. ����ȭ�� �Ǿ� �ֱ⿡ ū ������ ����.
	BuildStackedBlockPool(_device, _descHeap);
}

Grid::~Grid()
{
	for (int z = (Z_SIZE - 1); z >= 0; --z) {
		for (UINT y = 0; y < Y_SIZE; ++y) {
			for (UINT x = 0; x < X_SIZE; ++x) {
				if (m_stackedBlocks[x][y][z] != nullptr) {
					m_stackedBlocks[x][y][z].reset();
				}
			}
		}
	}
}



void Grid::OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess)
{
	GameObject::UpdateConstantBuffer();
	GameObject::SetDescriptorTable(_device, _cmdList, _descHeap);

	// ���� Normal Shader�� ������ Grid�� �׸��� ����
	_cmdList->SetPipelineState(_pso);
	_cmdList->IASetVertexBuffers(0, 1, &m_pGeometry->GetVertexBufferView());
	_cmdList->IASetIndexBuffer(&m_pGeometry->GetIndexBufferView());
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	_cmdList->DrawIndexedInstanced(m_pGeometry->IndexNum, 1, 0, 0, 0);

	// Tessellation Shader�� ������ StackedBlock�� �׸��� ���� Current�� NextBlock�� ��� �׸���.
	OnRender_StackedBlock(_device, _cmdList, _descHeap, _pso, _psoTess);
}

void Grid::BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
	vector<Vertex> vertices;
	vertices.reserve(90);

	// ����
	for (float i = (float)X_SIZE; i >= 0; --i) {
		vertices.push_back(Vertex{ XMFLOAT3(0.0f, i, (float)Z_SIZE), XMFLOAT4(Colors::Green) });
		vertices.push_back(Vertex{ XMFLOAT3((float)X_SIZE, i, (float)Z_SIZE), XMFLOAT4(Colors::Green) });

		vertices.push_back(Vertex{ XMFLOAT3(i, (float)Y_SIZE, (float)Z_SIZE), XMFLOAT4(Colors::Green) });
		vertices.push_back(Vertex{ XMFLOAT3(i, 0.0f, (float)Z_SIZE), XMFLOAT4(Colors::Green) });
	}
	// ����
	for (float i = (float)Z_SIZE - 1; i >= 0; --i) {
		vertices.push_back(Vertex{ XMFLOAT3(0.0f, (float)Y_SIZE, i), XMFLOAT4(Colors::Green) });
		vertices.push_back(Vertex{ XMFLOAT3(0.0f, 0.0f, i), XMFLOAT4(Colors::Green) });

	}
	for (float i = (float)Y_SIZE; i >= 0.0f; --i) {
		vertices.push_back(Vertex{ XMFLOAT3(0.0f, i, 0.0f), XMFLOAT4(Colors::Green) });
	}
	// ����
	for (float i = (float)Z_SIZE - 1; i >= 0; --i) {
		vertices.push_back(Vertex{ XMFLOAT3((float)X_SIZE, (float)Y_SIZE, i), XMFLOAT4(Colors::Green) });
		vertices.push_back(Vertex{ XMFLOAT3((float)X_SIZE, 0.0f, i), XMFLOAT4(Colors::Green) });

	}
	for (float i = (float)Y_SIZE; i >= 0.0f; --i) {
		vertices.push_back(Vertex{ XMFLOAT3((float)X_SIZE, i, 0.0f), XMFLOAT4(Colors::Green) });
	}
	// ���
	for (float i = 1.0f; i <= (float)X_SIZE - 1; ++i) {
		vertices.push_back(Vertex{ XMFLOAT3(i, (float)Y_SIZE, 0.0f), XMFLOAT4(Colors::Green) });
	}
	// �ϴ�
	for (float i = 1.0f; i <= (float)X_SIZE - 1; ++i) {
		vertices.push_back(Vertex{ XMFLOAT3(i, 0.0f, 0.0f), XMFLOAT4(Colors::Green) });
	}

	array<UINT, 94> indices = {
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 
		 0, 48,	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
		64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 
		76, 77,  1, 78,  5, 79,  9, 80, 13, 81, 17, 82, 
		21, 83,	24, 54, 26, 56, 28, 58, 30, 60, 32, 62, 
		34, 64, 36, 66, 38, 68, 40, 70, 42, 72, 44, 74, 
		46, 76,  6, 87, 10, 86, 14, 85, 18, 84
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

	m_pGeometry->VertexBuffer = D3DUtil::CreateDefaultBuffer(_device, _cmdList, vertices.data(), vbByteSize, m_pGeometry->VertexBufferUpload);
	m_pGeometry->IndexBuffer = D3DUtil::CreateDefaultBuffer(_device, _cmdList, indices.data(), ibByteSize, m_pGeometry->IndexBufferUpload);

	m_pGeometry->VertexByteStride = sizeof(Vertex);
	m_pGeometry->VBByteSize = vbByteSize;
	m_pGeometry->IBByteSize = ibByteSize;
	m_pGeometry->IndexNum = (UINT)indices.size();

	// StackedBlock�� Geometry Data�� �ʱ�ȭ�Ѵ�. �� 1ȸ�� �����Ѵ�.
	StackedBlock::BuildGeometry(_device, _cmdList);
}



UINT Grid::CalcRemoveNum()
{
	UINT num = 0;

	// �عٴں��� Ž���ϸ�
	for (int z = Z_SIZE - 1; z >= m_highestLayerOfStackedBlocks; --z) {
		// ������ �����ϴ� ���� �ִٸ�
		if (IsLayerFull(z)) {
			// ���� Block�� ��� �����ϰ�
			RemoveStackedBlocksOnLayer(z);
			// �ٽ� Ž���� �عٴں��� �ϵ��� �ϸ�
			++z;
			// Clear�� ���� ���� �����Ѵ�.
			++num;
		}
	}
	return num;
}

void Grid::ResetStackedBlocksData()
{
	// ���Ρ漼�Ρ���̸� ��� Ž���ϸ�
	for (int z = Z_SIZE - 1; z >= 0; --z) {
		for (UINT y = 0; y < Y_SIZE; ++y) {
			for (UINT x = 0; x < X_SIZE; ++x) {
				// �ش� ���� Block�� �ִٸ� FALSE�� �����.
				if (m_stackedBlocks[x][y][z]->m_isActivate == true) {
					m_stackedBlocks[x][y][z]->m_isActivate = false;
				}
			}
		}
	}
	m_highestLayerOfStackedBlocks = Z_SIZE;
}



void Grid::BuildStackedBlockPool(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
{
	for (int z = (Z_SIZE - 1); z >= 0; --z) {
		for (UINT y = 0; y < Y_SIZE; ++y) {
			for (UINT x = 0; x < X_SIZE; ++x) {
				if (m_stackedBlocks[x][y][z] == nullptr) {
					m_stackedBlocks[x][y][z] = make_unique<StackedBlock>(_device, _descHeap);
					m_stackedBlocks[x][y][z]->SetPosition(x, y, z);
				}
			}
		}
	}
}

void Grid::OnRender_StackedBlock(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso, ID3D12PipelineState* _psoTess) const
{
	// �ֻ������� Grid ���θ� ���� Ž���ϸ鼭
	for (int z = (Z_SIZE - 1); z >= 0; --z) {
		for (UINT y = 0; y < Y_SIZE; ++y) {
			for (UINT x = 0; x < X_SIZE; ++x) {
				// �ش� ��ġ�� Block�� �ִٸ� Block�� �׷��� �Ѵ�.
				if (IsStackedBlockAtLocation(x, y, z)) {
					m_stackedBlocks[x][y][z]->OnRender(_device, _cmdList, _descHeap, _pso, _psoTess);
				}
			}
		}
	}
}

void Grid::RemoveStackedBlocksOnLayer(UINT _layer)
{
	for (int z = _layer; z >= m_highestLayerOfStackedBlocks; --z) {
		for (UINT y = 0; y < Y_SIZE; ++y) {
			for (UINT x = 0; x < X_SIZE; ++x) {
				if (m_stackedBlocks[x][y][z - 1]->m_isActivate == false) {
					m_stackedBlocks[x][y][z]->m_isActivate = false;
				}
			}
		}
	}

	// �ְ����� ������ �� ĭ ������.
	++m_highestLayerOfStackedBlocks;
}

bool Grid::IsLayerFull(UINT _layer) const
{
	// ���Ρ漼�θ� ��� Ž���ϸ�
	for (UINT y = 0; y < Y_SIZE; ++y) {
		for (UINT x = 0; x < X_SIZE; ++x) {

			// �ش� ���� Block�� �� ���� ���ٸ� False�� ��ȯ.
			if (!IsStackedBlockAtLocation(x, y, _layer)) {
				return false;
			}
		}
	}
	// �ش� ���� Block�� ��� �ִٸ� True�� ��ȯ.
	return true;
}



void Grid::StackBlockAtLocation(UINT _x, UINT _y, UINT _z)
{
	if (!(_x >= 0 && _x < X_SIZE) ||
		!(_y >= 0 && _y < Y_SIZE) ||
		!(_z >= 1 && _z < Z_SIZE) ||
		m_stackedBlocks[_x][_y][_z]->m_isActivate) {
		return;
	}

	if (_z < m_highestLayerOfStackedBlocks) {
		m_highestLayerOfStackedBlocks = _z;
	}

	m_stackedBlocks[_x][_y][_z]->m_isActivate = true;
}

bool Grid::IsStackedBlockAtLocation(UINT _x, UINT _y, UINT _z)
{
	assert(_x >= 0 && _x < X_SIZE);
	assert(_y >= 0 && _y < Y_SIZE);
	assert(_z >= 0 && _z < Z_SIZE);

	return m_stackedBlocks[_x][_y][_z]->m_isActivate;
}

UINT Grid::m_highestLayerOfStackedBlocks = Z_SIZE;
unique_ptr<StackedBlock> Grid::m_stackedBlocks[X_SIZE][Y_SIZE][Z_SIZE] = { nullptr };