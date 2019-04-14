#include "stdafx.h"
#include "Block.h"
#include "Grid.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

Block::Block(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:GameObject(_device, _descHeap) {}

void Block::StackInGrid() {
	XMVECTOR pos = XMLoadFloat3(&m_position);

	XMVECTOR correct = { 0.5f, 0.5f, 0.5f };
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		relative -= correct;
 		relative += pos;
 		Grid::StackBlockAtLocation(floor(XMVectorGetX(relative) + 0.5f), floor(XMVectorGetY(relative) + 0.5f), floor(XMVectorGetZ(relative) + 0.5f));
	}
}

bool Block::TryTranslate(float _x, float _y, float _z) {
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR trans = XMVectorSet(_x, _y, _z, 1.0f);

	XMVECTOR correct = { 0.5f, 0.5f, 0.5f };
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		relative -= correct;
		relative += pos;
		relative += trans;
		if (!IsMovable(relative)) {
			return false;
		}
	}

	pos += trans;
	SetPosition(pos);

	return true;
}

bool Block::TryRotate(float _pitch, float _yaw, float _roll) {
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR qua = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, _roll);
	XMMATRIX rotMat = XMMatrixRotationQuaternion(qua);

	XMVECTOR correct = { 0.5f, 0.5f, 0.5f };
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		relative = XMVector3TransformCoord(relative, rotMat);
		relative -= correct;
		relative += pos;
		if (!IsMovable(relative)) {
			return false;
		}
	}

	Rotate(_pitch, _yaw, _roll);

	for (auto &a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		relative = XMVector3TransformCoord(relative, rotMat);
		XMStoreFloat3(&a, relative);
	}

	return true;
}

bool Block::IsMovable(const DirectX::XMVECTOR& _temp) const {
	if (floor(XMVectorGetX(_temp) + 0.5f) >= 0 && floor(XMVectorGetX(_temp) + 0.5f) < X_SIZE &&
		floor(XMVectorGetY(_temp) + 0.5f) >= 0 && floor(XMVectorGetY(_temp) + 0.5f) < Y_SIZE &&
		floor(XMVectorGetZ(_temp) + 0.5f) < Z_SIZE &&
		!(Grid::IsStackedBlockAtLocation(floor(XMVectorGetX(_temp) + 0.5f), floor(XMVectorGetY(_temp) + 0.5f), floor(XMVectorGetZ(_temp) + 0.5f)))) {
		return true;
	}
	return false;
}