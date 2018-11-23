#pragma once
#include "stdafx.h"

class Block;

class BlockFactory final {
public:
	static void BuildBlockFactory(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap);
	static std::shared_ptr<Block> SelectRandomBlock();

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