#pragma once
#include "stdafx.h"
#include "GameObject.h"

class StackedBlock final : public GameObject {
public:
	StackedBlock(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~StackedBlock() = default;

	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso) override;

	static void BuildGeometry_S(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList);

private:
#pragma region _NotUsed_
	virtual void BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) override {};
	virtual void BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) override {};
#pragma endregion This class use static Build method for optimization
	static void BuildBorder_S(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList);

public:
	bool m_isActivate = false;

private:
	static std::unique_ptr<MeshGeometry> m_pGeometry_S;
	static std::unique_ptr<MeshGeometry> m_pBorder_S;
};



class Grid final : public GameObject {
public:
	Grid(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~Grid() = default;

	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso) override;
	virtual void BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) override;

	UINT CalcRemovedNumOfLayer();
	void ResetStackedBlocksData();

private:
#pragma region _NotUsed_
	virtual void BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) override {};
#pragma endregion Grid does not need a border
	void BuildStackedBlockPool(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	void OnRender_StackedBlock(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) const;
	void RemoveStackedBlocksOnLayer(UINT _layer);
	bool IsLayerFull(UINT _layer) const;

public:
	static void StackBlockAtLocation(UINT _x, UINT _y, UINT _z);
	static bool IsStackedBlockAtLocation(UINT _x, UINT _y, UINT _z);

public:
	static UINT m_highestLayerOfStackedBlocks;

private:
	static std::unique_ptr<StackedBlock> m_stackedBlocks[X_SIZE][Y_SIZE][Z_SIZE];
};