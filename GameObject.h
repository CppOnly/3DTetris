#pragma once
#include "stdafx.h"
#include "D3D12Helper.h"

class GameObject
{
public:
	GameObject(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	virtual ~GameObject();

	// Object�� �⺻ ������ �������, ���Կ����� �����մϴ�.
	GameObject() = delete;
	GameObject(const GameObject& rhs) = delete;
	GameObject& operator=(const GameObject& rhs) = delete;

public:
	virtual void OnRender(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap, ID3D12PipelineState* _pso = nullptr, ID3D12PipelineState* _psoTess = nullptr) = 0;
	// ���� ������ ������ �׸��� �ʹٸ� �� �Լ��� ���� Geometry�� �����Ͻʽÿ�.
	virtual void BuildGeometry(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) {};

	// SRT Get Method.
	DirectX::XMMATRIX& GetWorldMatrix() const;
	DirectX::XMVECTOR& GetScale() const;
	DirectX::XMVECTOR& GetPosition() const;
	DirectX::XMVECTOR& GetQuaternion() const;

	// SRT Set Method.
	void Scale(float _x, float _y, float _z);
	void Scale(const DirectX::XMVECTOR& _scale);

	void SetScale(float _x, float _y, float _z);
	void SetScale(const DirectX::XMVECTOR& _scale);

	void Translate(float _x, float _y, float _z);
	void Translate(const DirectX::XMVECTOR& _trans);

	void SetPosition(float _x, float _y, float _z);
	void SetPosition(const DirectX::XMVECTOR& _pos);

	void Rotate(float _pitch, float _yaw, float _roll);
	void Rotate(const DirectX::XMMATRIX& _rot);
	void Rotate(const DirectX::XMVECTOR& _qua);
	void Rotate(const DirectX::XMVECTOR& _axis, float _angle);

	void SetRotation(float _pitch, float _yaw, float _roll);
	void SetRotation(const DirectX::XMMATRIX& _rot);
	void SetRotation(const DirectX::XMVECTOR& _qua);
	void SetRotation(const DirectX::XMVECTOR& _axis, float _angle);
	
protected:
	// ���� ������ �׵θ��� �׸��� �ʹٸ� �� �Լ��� ���� BorderGeometry�� �����Ͻʽÿ�.
	virtual void BuildBorder(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList) {};
	// Update Constant Data.
	void UpdateConstantBuffer();
	// SetDescriptorTable for DrawCall.
	void SetDescriptorTable(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap);

private:
	// ObjectConstantBuffer ����.
	void BuildConstantBuffer(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap);
	
protected:
	// Object�� Geometry Data.
	std::unique_ptr<MeshGeometry> m_pGeometry = nullptr;
	std::unique_ptr<MeshGeometry> m_pBorder = nullptr;
	// Object�� Constant Data.
	std::unique_ptr<D3DUtil::UploadBuffer<ObjectConstant>> m_constantBuffer = nullptr;

	// Object�� SRT Data.
	DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 m_quaternion = { 0.0f, 0.0f, 0.0f, 1.0f };

	// CBVHeap���� �ڽ��� ConstantBuffer�� ã�� ���ִ� ����.
	UINT m_gameObjectIndex = 0;

protected:
	static UINT m_gameObjectsNum;
};