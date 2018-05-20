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

	// Block을 이루는 각각의 세부Block들의 중점에 대해
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		// Position을 좌상단모퉁이로 보정해주고
		relative -= cor;
		// 각 Position에 현재 Block의 이동 수치(m_position)를 더해주면 최종 World Position이 나온다.
 		relative += pos;
		// Gird내 해당 위치에 Block을 쌓으면 된다.
 		Grid::StackBlockAtLocation(floor(XMVectorGetX(relative) + 0.5f), floor(XMVectorGetY(relative) + 0.5f), floor(XMVectorGetZ(relative) + 0.5f));
	}
}

bool Block::TryTranslate(float _x, float _y, float _z)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR cor = XMLoadFloat3(&m_correction);
	// 이동 수치
	XMVECTOR trans = XMVectorSet(_x, _y, _z, 1.0f);

	// Block을 이루는 각각의 세부Block들의 중점에 대해
	for (auto a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		// Position을 좌상단모퉁이로 보정해주고
		relative -= cor;
		// 각 Position에 현재 Block의 이동 수치(m_position)를 더해주면 최종 World Position이 나온다.
		relative += pos;
		// 그리고 위 값에 이동 수치를 더해주면 이동된 World Position이 나온다.
		relative += trans;

		// 세부 Block들의 이동된 값이 허용 범위를 한 개라도 벗어난다면 FALSE를 반환하고 종료된다.
		if (!IsMovable(relative)) {
			return false;
		}
	}

	// 반면 위 과정에서 함수가 종료되지 않았다면 해당 이동은 유효한 이동이라는 뜻이므로 Position을 변경한다.
	pos += trans;
	SetPosition(pos);

	return true;
}

bool Block::TryRotate(float _pitch, float _yaw, float _roll)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR cor = XMLoadFloat3(&m_correction);
	// 회전 변환
	XMVECTOR qua = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, _roll);
	XMMATRIX rot = XMMatrixRotationQuaternion(qua);

	// Block을 이루는 각각의 세부Block들의 중점에 대해
	for (auto &a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		// Translate와 살짝 다른점은 반드시 먼저 회전을 해주고 보정을 해줘야 한다는 것이다.
		relative = XMVector3TransformCoord(relative, rot);
		// Position을 좌상단모퉁이로 보정해주고
		relative -= cor;
		// 각 Position에 현재 Block의 이동 수치(m_position)를 더해주면 최종 World Position이 나온다.
		relative += pos;

		// 세부 Block들의 이동된 값이 허용 범위를 한 개라도 벗어난다면 FALSE를 반환하고 종료된다.
		if (!IsMovable(relative)) {
			return false;
		}
	}

	// 반면 위 과정에서 함수가 종료되지 않았다면 해당 회전은 유효한 회전이라는 뜻이므로 회전 변환을 변경한다.
	Rotate(_pitch, _yaw, _roll);

	// Translate와 다르게 Rotate는 연관Block의 위치도 수정을 해줘야 한다.
	for (auto &a : m_blockRelativePositions) {
		XMVECTOR relative = XMLoadFloat3(&a);
		relative = XMVector3TransformCoord(relative, rot);
		XMStoreFloat3(&a, relative);
	}

	return true;
}



bool Block::IsMovable(const XMVECTOR& _temp) const
{
	// 이동/회전 가능 기준은 다음과 같다. : Position이 Gird 내부에 위치하거나 or 해당 위치에 쌓인 Block이 없거나.
	if (floor(XMVectorGetX(_temp) + 0.5f) >= 0 && floor(XMVectorGetX(_temp) + 0.5f) < X_SIZE &&
		floor(XMVectorGetY(_temp) + 0.5f) >= 0 && floor(XMVectorGetY(_temp) + 0.5f) < Y_SIZE &&
		floor(XMVectorGetZ(_temp) + 0.5f) < Z_SIZE &&
		!(Grid::IsStackedBlockAtLocation(floor(XMVectorGetX(_temp) + 0.5f), floor(XMVectorGetY(_temp) + 0.5f), floor(XMVectorGetZ(_temp) + 0.5f)))) {
		return true;
	}

	return false;
}