cbuffer ObjectConstantBuffer : register(b0) {
	float4x4 WorldMat;
};

cbuffer SceneConstantBuffer : register(b1) {
	float4x4 ViewProjMat;
	float3   EyePosW;
};

struct VertexIn {
	float3 PosL  : POSITION;
	float4 Color : COLOR;
};

struct VertexOut {
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn _vin) {
	VertexOut vout;

	vout.PosH = mul(float4(_vin.PosL, 1.0f), mul(WorldMat, ViewProjMat));

	vout.Color = _vin.Color;

	return vout;
}

float4 PS(VertexOut _pin) : SV_Target {
	return _pin.Color;
}