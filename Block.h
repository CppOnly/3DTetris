#pragma once
#include "stdafx.h"
#include "GameObject.h"

class Block : public GameObject {
public:
	Block(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~Block() = default;

	virtual void ResetRelativeData() = 0;

	void StackInGrid();
	bool TryTranslate(float _x, float _y, float _z);
	bool TryRotate(float _pitch, float _yaw, float _roll);

private:
	bool IsMovable(const DirectX::XMVECTOR& _temp) const;

protected:
	std::vector<DirectX::XMFLOAT3> m_blockRelativePositions;
};