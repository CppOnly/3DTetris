#include "stdafx.h"
#include "D3D12Helper.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

DxException::DxException(HRESULT _hr, const wstring& _functionName, const std::wstring& _filename, int _lineNumber)
	:ErrorCode(_hr), FunctionName(_functionName), Filename(_filename), LineNumber(_lineNumber) {}

std::wstring DxException::ToString()const {
	_com_error err(ErrorCode);
	std::wstring msg = err.ErrorMessage();

	return FunctionName + L"failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error : " + msg;
}



UINT D3DUtil::CalcConstantBufferByteSize(UINT _byteSize) {
	UINT temp = _byteSize / 256;
	return 256 * (temp + 1);
}

ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const void* _initData, UINT64 _byteSize, ComPtr<ID3D12Resource>& _uploadBuffer) {
	ComPtr<ID3D12Resource> defaultBuffer;

	ThrowIfFailed(_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(_byteSize), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(defaultBuffer.GetAddressOf())));
	ThrowIfFailed(_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(_byteSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(_uploadBuffer.GetAddressOf())));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = _initData;
	subResourceData.RowPitch = _byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(_cmdList, defaultBuffer.Get(), _uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	_cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	return defaultBuffer;
}

ComPtr<ID3DBlob> D3DUtil::CompileShader(const std::wstring& _filename, const D3D_SHADER_MACRO* _pDefines, const string& _entrypoint, const std::string& _target) {
	UINT compileFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(_filename.c_str(), _pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, _entrypoint.c_str(), _target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr) {
		OutputDebugStringA((char*)errors->GetBufferPointer());
	}
	return byteCode;
}



D3D12_VERTEX_BUFFER_VIEW MeshGeometry::GetVertexBufferView() const {
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
	vbv.StrideInBytes = VertexByteStride;
	vbv.SizeInBytes = VBByteSize;

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW MeshGeometry::GetIndexBufferView() const {
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	ibv.Format = DXGI_FORMAT_R32_UINT;
	ibv.SizeInBytes = IBByteSize;

	return ibv;
}