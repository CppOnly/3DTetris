#pragma once
#include "stdafx.h"
#include "GameObject.h"

class StackedBlock : public GameObject
{
public:
	StackedBlock(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~StackedBlock();

	StackedBlock() = delete;
	StackedBlock(const StackedBlock& rhs) = delete;
	StackedBlock& operator=(const StackedBlock& rhs) = delete;

public:
	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) override;

public:
	bool m_isActivate = false;

public:
	static void BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList);

private:
	static void BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList);

private:
	// 300여개의 객체가 동일한 Data를 사용하므로 최적화를 위해 Static Geometry Data를 따로 쓴다.
	static std::unique_ptr<MeshGeometry> m_pGeometry;
	static std::unique_ptr<MeshGeometry> m_pBorder;
};





class Grid : public GameObject
{
public:
	Grid(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~Grid();

	Grid() = delete;
	Grid(const Grid& rhs) = delete;
	Grid& operator=(const Grid& rhs) = delete;

public:
	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) override;
	virtual void BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) override;

	// Clear한 층의 개수를 계산.
	UINT CalcRemoveNum();
	// 모든 Stack 정보를 초기화
	void ResetStackedBlocksData();

private:
	void BuildStackedBlockPool(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	// Stack여부를 토대로 StackedBlock의 Rendering을 활성화.
	void OnRender_StackedBlock(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) const;
	// Clear시 비활성화할 StackedBlock 결정.
	void RemoveStackedBlocksOnLayer(UINT _layer);
	// 층이 Clear될 수 있는지를 계산.
	bool IsLayerFull(UINT _layer) const;

public:
	static void StackBlockAtLocation(UINT _x, UINT _y, UINT _z);
	static bool IsStackedBlockAtLocation(UINT _x, UINT _y, UINT _z);

public:
	static UINT m_highestLayerOfStackedBlocks;

private:
	static std::unique_ptr<StackedBlock> m_stackedBlocks[X_SIZE][Y_SIZE][Z_SIZE];
};