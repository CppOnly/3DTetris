#pragma once
#include "stdafx.h"
#include "D3D12Helper.h"

class GameObject {
public:
	GameObject(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~GameObject() = default;

	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso) = 0;
	virtual void BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) = 0;

	DirectX::XMMATRIX GetWorldMatrix() const;
	DirectX::XMVECTOR GetScale() const;
	DirectX::XMVECTOR GetPosition() const;
	DirectX::XMVECTOR GetQuaternion() const;

	void Scaling(float _x, float _y, float _z);
	void Scaling(const DirectX::XMVECTOR& _scaleSource);
	void SetScale(float _x, float _y, float _z);
	void SetScale(const DirectX::XMVECTOR& _scale);

	void Rotate(float _pitch, float _yaw, float _roll);
	void Rotate(const DirectX::XMMATRIX& _rotMat);
	void Rotate(const DirectX::XMVECTOR& _quaSource);
	void Rotate(const DirectX::XMVECTOR& _axis, float _angle);
	void SetQuaternion(float _pitch, float _yaw, float _roll);
	void SetQuaternion(const DirectX::XMMATRIX& _rotMat);
	void SetQuaternion(const DirectX::XMVECTOR& _qua);
	void SetQuaternion(const DirectX::XMVECTOR& _axis, float _angle);

	void Translate(float _x, float _y, float _z);
	void Translate(const DirectX::XMVECTOR& _trans);
	void SetPosition(float _x, float _y, float _z);
	void SetPosition(const DirectX::XMVECTOR& _pos);

protected:
	virtual void BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) = 0;
	void UpdateConstantBuffer();
	void SetDescriptorTable(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap);

private:
	void BuildConstantBuffer(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	
protected:
	std::unique_ptr<MeshGeometry> m_pGeometry = nullptr;
	std::unique_ptr<MeshGeometry> m_pBorder = nullptr;
	std::unique_ptr<D3DUtil::UploadBuffer<ObjectConstant>> m_constantBuffer = nullptr;

	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 m_quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };

private:
	UINT m_gameObjectIndex = 0;

	static UINT m_gameObjectsNum;
#pragma region _Forbidden Constructor_
private:
	GameObject() = delete;
	GameObject(const GameObject& rhs) = delete;
	GameObject& operator=(const GameObject& rhs) = delete;
#pragma endregion
};