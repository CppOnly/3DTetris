#pragma once
#include "stdafx.h"
#include "GameObject.h"

class Block : public GameObject
{
public:
	Block(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~Block() = default;

	Block() = delete;
	Block(const Block& rhs) = delete;
	Block& operator=(const Block& rhs) = delete;

public:
	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) = 0;
	virtual void ResetRelativeData() = 0;

	virtual void StackInGrid();
	virtual bool TryTranslate(float _x, float _y, float _z);
	virtual bool TryRotate(float _pitch, float _yaw, float _roll);

protected:
	virtual bool IsMovable(const DirectX::XMVECTOR& _temp) const;

protected:
	// Block을 이루는 세부Block들의 중점의 위치(연관)를 담는 변수.
	std::vector<DirectX::XMFLOAT3> m_blockRelativePositions;
	DirectX::XMFLOAT3 m_correction = { 0.5f, 0.5f, 0.5f };
};