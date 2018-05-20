#include "stdafx.h"
#include "GameObject.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

GameObject::GameObject(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
{
	m_pGeometry = make_unique<MeshGeometry>();
	m_pBorder = make_unique<MeshGeometry>();
	m_constantBuffer = make_unique<D3DUtil::UploadBuffer<ObjectConstant>>(_device, 1, true);

	++m_gameObjectsNum;
	m_gameObjectIndex = m_gameObjectsNum;

	BuildConstantBuffer(_device, _descHeap);
}

GameObject::~GameObject()
{
	m_pGeometry.reset();
	m_pBorder.reset();
	m_constantBuffer.reset();
}



XMMATRIX& GameObject::GetWorldMatrix() const
{
	XMMATRIX scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	XMVECTOR qua = XMLoadFloat4(&m_quaternion);
	XMMATRIX rot = XMMatrixRotationQuaternion(qua);
	XMMATRIX trans = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	XMMATRIX world = scale * rot * trans;

	return world;
}

XMVECTOR& GameObject::GetScale() const
{
	return XMLoadFloat3(&m_scale);
}

XMVECTOR& GameObject::GetPosition() const
{
	return XMLoadFloat3(&m_position);
}

XMVECTOR& GameObject::GetQuaternion() const
{
	return XMLoadFloat4(&m_quaternion);
}



void GameObject::Scale(float _x, float _y, float _z)
{
	assert(!(0.0f == _x || 0.0f == _y || 0.0f == _z));

	XMVECTOR scaleD = XMLoadFloat3(&m_scale);
	XMVECTOR scaleS = XMVectorSet(_x, _y, _z, 1.0f);
	scaleD *= scaleS;
	XMStoreFloat3(&m_scale, scaleD);
}

void GameObject::Scale(const XMVECTOR& _scale)
{
	assert(!(0.0f == XMVectorGetX(_scale) || 0.0f == XMVectorGetY(_scale) || 0.0f == XMVectorGetZ(_scale)));
	
	XMVECTOR scale = XMLoadFloat3(&m_scale);
	scale *= _scale;
	XMStoreFloat3(&m_scale, scale);
}



void GameObject::SetScale(float _x, float _y, float _z)
{
	assert(!(0.0f == _x || 0.0f == _y || 0.0f == _z));

	XMVECTOR scale = XMVectorSet(_x, _y, _z, 1.0f);
	XMStoreFloat3(&m_scale, scale);
}

void GameObject::SetScale(const XMVECTOR& _scale)
{
	assert(!(0.0f == XMVectorGetX(_scale) || 0.0f == XMVectorGetY(_scale) || 0.0f == XMVectorGetZ(_scale)));

	XMStoreFloat3(&m_scale, _scale);
}



void GameObject::Translate(float _x, float _y, float _z)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR trans = XMVectorSet(_x, _y, _z, 1.0f);
	pos += trans;
	XMStoreFloat3(&m_position, pos);
}

void GameObject::Translate(const XMVECTOR& _trans)
{
	XMVECTOR pos = XMLoadFloat3(&m_position);
	pos += _trans;
	XMStoreFloat3(&m_position, pos);
}



void GameObject::SetPosition(float _x, float _y, float _z)
{
	XMVECTOR pos = XMVectorSet(_x, _y, _z, 1.0f);
	XMStoreFloat3(&m_position, pos);
}

void GameObject::SetPosition(const XMVECTOR& _pos)
{
	XMStoreFloat3(&m_position, _pos);
}



void GameObject::Rotate(float _pitch, float _yaw, float _roll)
{
	XMVECTOR quaD = XMLoadFloat4(&m_quaternion);
	XMVECTOR quaS = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, _roll);
	quaD = XMQuaternionMultiply(quaD, quaS);
	XMStoreFloat4(&m_quaternion, quaD);
}

void GameObject::Rotate(const XMMATRIX& _rot)
{
	XMVECTOR quaD = XMLoadFloat4(&m_quaternion);
	XMVECTOR quaS = XMQuaternionRotationMatrix(_rot);
	quaD = XMQuaternionMultiply(quaD, quaS);
	XMStoreFloat4(&m_quaternion, quaD);
}

void GameObject::Rotate(const XMVECTOR& _qua)
{
	XMVECTOR qua = XMLoadFloat4(&m_quaternion);
	qua = XMQuaternionMultiply(qua, _qua);
	XMStoreFloat4(&m_quaternion, qua);
}

void GameObject::Rotate(const XMVECTOR& _axis, float _angle)
{
	XMVECTOR qua = XMLoadFloat4(&m_quaternion);
	XMVECTOR rot = XMQuaternionRotationAxis(_axis, _angle);
	qua = XMQuaternionMultiply(qua, rot);
	XMStoreFloat4(&m_quaternion, qua);
}



void GameObject::SetRotation(float _pitch, float _yaw, float _roll)
{
	XMVECTOR qua = XMQuaternionRotationRollPitchYaw(_pitch, _yaw, _roll);
	XMStoreFloat4(&m_quaternion, qua);
}

void GameObject::SetRotation(const XMMATRIX& _rot)
{
	XMVECTOR qua = XMQuaternionRotationMatrix(_rot);
	XMStoreFloat4(&m_quaternion, qua);
}

void GameObject::SetRotation(const XMVECTOR& _qua)
{
	XMStoreFloat4(&m_quaternion, _qua);
}

void GameObject::SetRotation(const XMVECTOR& _axis, float _angle)
{
	XMVECTOR qua = XMQuaternionRotationAxis(_axis, _angle);
	XMStoreFloat4(&m_quaternion, qua);
}



void GameObject::UpdateConstantBuffer()
{
	ObjectConstant constant;
	XMStoreFloat4x4(&constant.WorldMat, XMMatrixTranspose(GetWorldMatrix()));
	m_constantBuffer->CopyData(0, constant);
}

void GameObject::SetDescriptorTable(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, ID3D12DescriptorHeap* _descHeap)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(_descHeap->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(m_gameObjectIndex, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	_cmdList->SetGraphicsRootDescriptorTable(0, handle);
}



void GameObject::BuildConstantBuffer(ID3D12Device* _device, ID3D12DescriptorHeap* _descHeap)
{
	D3D12_GPU_VIRTUAL_ADDRESS address = m_constantBuffer->GetUploadBuffer()->GetGPUVirtualAddress();
	UINT size = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstant));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = address;
	cbvDesc.SizeInBytes = size;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_descHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(m_gameObjectIndex, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

	_device->CreateConstantBufferView(&cbvDesc, handle);
}

UINT GameObject::m_gameObjectsNum = 1;