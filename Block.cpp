#include "stdafx.h"
#include "Block.h"

#include "Grid.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

Block::Block(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
	:GameObject(_device, _descHeap)
{}



void Block::StackInGrid()
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR cor = XMLoadFloat3(&m_correction);

	// Block�� �̷�� ������ ����Block���� ������ ����
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		// Position�� �»�ܸ����̷� �������ְ�
		relative -= cor;
		// �� Position�� ���� Block�� �̵� ��ġ(m_position)�� �����ָ� ���� World Position�� ���´�.
 		relative += pos;
		// Gird�� �ش� ��ġ�� Block�� ������ �ȴ�.
 		Grid::StackBlockAtLocation(floor(XMVectorGetX(relative) + 0.5f), floor(XMVectorGetY(relative) + 0.5f), floor(XMVectorGetZ(relative) + 0.5f));
	}
}

bool Block::TryTranslate(float _x, float _y, float _z)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR cor = XMLoadFloat3(&m_correction);
	// �̵� ��ġ
	XMVECTOR trans = XMVectorSet(_x, _y, _z, 1.0f);

	// Block�� �̷�� ������ ����Block���� ������ ����
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		// Position�� �»�ܸ����̷� �������ְ�
		relative -= cor;
		// �� Position�� ���� Block�� �̵� ��ġ(m_position)�� �����ָ� ���� World Position�� ���´�.
		relative += pos;
		// �׸��� �� ���� �̵� ��ġ�� �����ָ� �̵��� World Position�� ���´�.
		relative += trans;

		// ���� Block���� �̵��� ���� ��� ������ �� ���� ����ٸ� FALSE�� ��ȯ�ϰ� ����ȴ�.
		if (!IsMovable(relative)) {
			return false;
		}
	}

	// �ݸ� �� �������� �Լ��� ������� �ʾҴٸ� �ش� �̵��� ��ȿ�� �̵��̶�� ���̹Ƿ� Position�� �����Ѵ�.
	pos += trans;
	SetPosition(pos);

	return true;
}

bool Block::TryRotate(float _pitch, float _yaw, float _roll)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR cor = XMLoadFloat3(&m_correction);
	// ȸ�� ��ȯ
	XMVECTOR qua = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, _roll);
	XMMATRIX rot = XMMatrixRotationQuaternion(qua);

	// Block�� �̷�� ������ ����Block���� ������ ����
	for (auto &a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		// Translate�� ��¦ �ٸ����� �ݵ�� ���� ȸ���� ���ְ� ������ ����� �Ѵٴ� ���̴�.
		relative = XMVector3TransformCoord(relative, rot);
		// Position�� �»�ܸ����̷� �������ְ�
		relative -= cor;
		// �� Position�� ���� Block�� �̵� ��ġ(m_position)�� �����ָ� ���� World Position�� ���´�.
		relative += pos;

		// ���� Block���� �̵��� ���� ��� ������ �� ���� ����ٸ� FALSE�� ��ȯ�ϰ� ����ȴ�.
		if (!IsMovable(relative)) {
			return false;
		}
	}

	// �ݸ� �� �������� �Լ��� ������� �ʾҴٸ� �ش� ȸ���� ��ȿ�� ȸ���̶�� ���̹Ƿ� ȸ�� ��ȯ�� �����Ѵ�.
	Rotate(_pitch, _yaw, _roll);

	// Translate�� �ٸ��� Rotate�� ����Block�� ��ġ�� ������ ����� �Ѵ�.
	for (auto &a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		relative = XMVector3TransformCoord(relative, rot);
		XMStoreFloat3(&a, relative);
	}

	return true;
}



bool Block::IsMovable(const XMVECTOR& _temp) const
{
	// �̵�/ȸ�� ���� ������ ������ ����. : Position�� Gird ���ο� ��ġ�ϰų� or �ش� ��ġ�� ���� Block�� ���ų�.
	if (floor(XMVectorGetX(_temp) + 0.5f) >= 0 && floor(XMVectorGetX(_temp) + 0.5f) < X_SIZE &&
		floor(XMVectorGetY(_temp) + 0.5f) >= 0 && floor(XMVectorGetY(_temp) + 0.5f) < Y_SIZE &&
		floor(XMVectorGetZ(_temp) + 0.5f) < Z_SIZE &&
		!(Grid::IsStackedBlockAtLocation(floor(XMVectorGetX(_temp) + 0.5f), floor(XMVectorGetY(_temp) + 0.5f), floor(XMVectorGetZ(_temp) + 0.5f)))) {
		return true;
	}

	return false;
}