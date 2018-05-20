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
	// 300������ ��ü�� ������ Data�� ����ϹǷ� ����ȭ�� ���� Static Geometry Data�� ���� ����.
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

	// Clear�� ���� ������ ���.
	UINT CalcRemoveNum();
	// ��� Stack ������ �ʱ�ȭ
	void ResetStackedBlocksData();

private:
	void BuildStackedBlockPool(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	// Stack���θ� ���� StackedBlock�� Rendering�� Ȱ��ȭ.
	void OnRender_StackedBlock(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) const;
	// Clear�� ��Ȱ��ȭ�� StackedBlock ����.
	void RemoveStackedBlocksOnLayer(UINT _layer);
	// ���� Clear�� �� �ִ����� ���.
	bool IsLayerFull(UINT _layer) const;

public:
	static void StackBlockAtLocation(UINT _x, UINT _y, UINT _z);
	static bool IsStackedBlockAtLocation(UINT _x, UINT _y, UINT _z);

public:
	static UINT m_highestLayerOfStackedBlocks;

private:
	static std::unique_ptr<StackedBlock> m_stackedBlocks[X_SIZE][Y_SIZE][Z_SIZE];
};