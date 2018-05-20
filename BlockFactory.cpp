#include "stdafx.h"
#include "BlockFactory.h"

#include "BlockTypeA.h"
#include "BlockTypeB.h"
#include "BlockTypeC.h"
#include "BlockTypeD.h"
#include "BlockTypeE.h"
#include "BlockTypeF.h"

using namespace std;

void BlockFactory::BuildAllBlockTypeGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap)
{
	srand(time(NULL));

	BlockTypeA::BuildGeometry(_device, _cmdList);
	BlockTypeB::BuildGeometry(_device, _cmdList);
	BlockTypeC::BuildGeometry(_device, _cmdList);
	BlockTypeD::BuildGeometry(_device, _cmdList);
	BlockTypeE::BuildGeometry(_device, _cmdList);
	BlockTypeF::BuildGeometry(_device, _cmdList);

	BuildAllBlockTypePool(_device, _descHeap);
}

void BlockFactory::DestoryBlockFactory()
{
	for (UINT i = 0; i < 2; ++i) {
		m_pBlockA[i].reset();	m_pBlockB[i].reset();
		m_pBlockC[i].reset();	m_pBlockD[i].reset();
		m_pBlockE[i].reset();	m_pBlockF[i].reset();
	}
}

shared_ptr<Block> BlockFactory::SelectRandomBlock()
{
	switch (rand() % m_blockTypeNym)
	{
	case 0:
		m_blockTypeAIndex = (m_blockTypeAIndex + 1) % 2;
		return m_pBlockA[m_blockTypeAIndex];
		break;

	case 1:
		m_blockTypeBIndex = (m_blockTypeBIndex + 1) % 2;
		return m_pBlockB[m_blockTypeBIndex];
		break;

	case 2:
		m_blockTypeCIndex = (m_blockTypeCIndex + 1) % 2;
		return m_pBlockC[m_blockTypeCIndex];
		break;

	case 3:
		m_blockTypeDIndex = (m_blockTypeDIndex + 1) % 2;
		return m_pBlockD[m_blockTypeDIndex];
		break;

	case 4:
		m_blockTypeEIndex = (m_blockTypeEIndex + 1) % 2;
		return m_pBlockE[m_blockTypeEIndex];
		break;

	case 5:
		m_blockTypeFIndex = (m_blockTypeFIndex + 1) % 2;
		return m_pBlockF[m_blockTypeFIndex];
		break;

	default:
		break;
	}
}



void BlockFactory::BuildAllBlockTypePool(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap) 
{
	for (UINT i = 0; i < 2; ++i) {
		m_pBlockA[i] = make_shared<BlockTypeA>(_device, _descHeap);
		m_pBlockB[i] = make_shared<BlockTypeB>(_device, _descHeap);
		m_pBlockC[i] = make_shared<BlockTypeC>(_device, _descHeap);
		m_pBlockD[i] = make_shared<BlockTypeD>(_device, _descHeap);
		m_pBlockE[i] = make_shared<BlockTypeE>(_device, _descHeap);
		m_pBlockF[i] = make_shared<BlockTypeF>(_device, _descHeap);
	}
}

UINT BlockFactory::m_blockTypeNym = 6;

UINT BlockFactory::m_blockTypeAIndex = 0;
UINT BlockFactory::m_blockTypeBIndex = 0;
UINT BlockFactory::m_blockTypeCIndex = 0;
UINT BlockFactory::m_blockTypeDIndex = 0;
UINT BlockFactory::m_blockTypeEIndex = 0;
UINT BlockFactory::m_blockTypeFIndex = 0;

shared_ptr<Block> BlockFactory::m_pBlockA[2] = { nullptr };
shared_ptr<Block> BlockFactory::m_pBlockB[2] = { nullptr };
shared_ptr<Block> BlockFactory::m_pBlockC[2] = { nullptr };
shared_ptr<Block> BlockFactory::m_pBlockD[2] = { nullptr };
shared_ptr<Block> BlockFactory::m_pBlockE[2] = { nullptr };
shared_ptr<Block> BlockFactory::m_pBlockF[2] = { nullptr };