#pragma once
#include "stdafx.h"

class Block;

// 각 Block별로 2개의 객체를 생성후 번갈아가면서 사용하는 방식으로 운용한다.
class BlockFactory
{
public:
	static void BuildAllBlockTypeGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap);
	static void DestoryBlockFactory();
	static std::shared_ptr<Block> SelectRandomBlock();

private:
	static void BuildAllBlockTypePool(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);

private:
	static UINT m_blockTypeNym;

	static UINT m_blockTypeAIndex;
	static UINT m_blockTypeBIndex;
	static UINT m_blockTypeCIndex;
	static UINT m_blockTypeDIndex;
	static UINT m_blockTypeEIndex;
	static UINT m_blockTypeFIndex;

	static std::shared_ptr<Block> m_pBlockA[2];
	static std::shared_ptr<Block> m_pBlockB[2];
	static std::shared_ptr<Block> m_pBlockC[2];
	static std::shared_ptr<Block> m_pBlockD[2];
	static std::shared_ptr<Block> m_pBlockE[2];
	static std::shared_ptr<Block> m_pBlockF[2];
};