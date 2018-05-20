#pragma once
#include "stdafx.h"

struct DxException
{
	DxException(HRESULT _hr, const std::wstring& _functionName, const std::wstring& _filename, int _lineNumber);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& _str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, _str.c_str(), -1, buffer, 512);

	return std::wstring(buffer);
}

inline void ThrowIfFailed(HRESULT _hr)
{
	if (FAILED(_hr)) {
		throw std::exception();
	}
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif





namespace D3DUtil
{
	UINT CalcConstantBufferByteSize(UINT _byteSize);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const void* _initData, UINT64 _byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& _uploadBuffer);

	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& _filename, const D3D_SHADER_MACRO* _pDefines, const std::string& _entrypoint, const std::string& _target);

	template<typename T>
	class UploadBuffer
	{
	public:
		UploadBuffer(ID3D12Device* _device, UINT _elementNum, bool _isConstantBuffer) 
			: m_isConstantBuffer(_isConstantBuffer)
		{
			if (_isConstantBuffer) {
				m_elementByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(T));
			}
			else {
				m_elementByteSize = sizeof(T);
			}
			ThrowIfFailed(_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * _elementNum), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_uploadBuffer)));
			ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pMappedData)));
		}
		~UploadBuffer()
		{
			if (m_uploadBuffer != nullptr) {
				m_uploadBuffer->Unmap(0, nullptr);
			}
			m_pMappedData = nullptr;
		}

		UploadBuffer() = delete;
		UploadBuffer(const UploadBuffer& rhs) = delete;
		UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

		ID3D12Resource* GetUploadBuffer() const
		{
			return m_uploadBuffer.Get();
		}

		void CopyData(int _elementIndex, const T& _data)
		{
			memcpy(&m_pMappedData[_elementIndex * m_elementByteSize], &_data, sizeof(T));
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer = nullptr;
		BYTE* m_pMappedData = nullptr;
		UINT  m_elementByteSize = 0;
		bool  m_isConstantBuffer = false;
	};
};





namespace Math
{
	const float PI = 3.1415926535f;

	template<typename T>
	T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	inline DirectX::XMFLOAT4X4 Identity4x4()
	{
		DirectX::XMFLOAT4X4 identity(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return identity;
	}
};





struct MeshGeometry
{
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUpload = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUpload = nullptr;

	UINT VertexByteStride = 0;
	UINT VBByteSize = 0;
	UINT IBByteSize = 0;
	UINT IndexNum = 0;
};

struct Vertex
{
	DirectX::XMFLOAT3 Pos = {0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct ObjectConstant
{
	DirectX::XMFLOAT4X4 WorldMat = Math::Identity4x4();
};

struct SceneConstant
{
	DirectX::XMFLOAT4X4 ViewProjMat = Math::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
};