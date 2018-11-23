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
	float4 PosL  : POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn _vin) {
	VertexOut vout;
	vout.PosL = float4(_vin.PosL, 1.0f);

	vout.Color = _vin.Color;

	return vout;
}

struct PatchTess {
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> _patch, uint _patchID : SV_PrimitiveID) {
	PatchTess pt;

	float3 centerL = 0.25f * (_patch[0].PosL + _patch[1].PosL + _patch[2].PosL + _patch[3].PosL);
	float3 centerW = mul(float4(centerL, 1.0f), WorldMat).xyz;

	float d = distance(centerW, EyePosW);

	const float d0 = 1.0f;
	const float d1 = 40.0f;
	float tess = 64.0f * saturate((d1 - d) / (d1 - d0));

	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.EdgeTess[3] = tess;

	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;

	return pt;
}

struct HullOut {
	float3 PosL  : POSITION;
	float4 Color : COLOR;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> _patch, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	HullOut hout;

	hout.PosL = _patch[i].PosL;
	hout.Color = _patch[i].Color;

	return hout;
}

struct DomainOut {
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

[domain("quad")]
DomainOut DS(const OutputPatch<HullOut, 4> _quad, PatchTess _patchTess, float2 _uv : SV_DomainLocation) {
	DomainOut dout;

	float3 v1 = lerp(_quad[0].PosL, _quad[1].PosL, _uv.x);
	float3 v2 = lerp(_quad[2].PosL, _quad[3].PosL, _uv.x);
	float3 p = lerp(v1, v2, _uv.y);

	float4 posW = mul(float4(p, 1.0f), WorldMat);
	dout.PosH = mul(posW, ViewProjMat);

	float4 v11 = lerp(_quad[0].Color, _quad[1].Color, _uv.x);
	float4 v22 = lerp(_quad[2].Color, _quad[3].Color, _uv.x);
	float4 c = lerp(v11, v22, _uv.y);

	dout.Color = c;

	return dout;
}

float4 PS(DomainOut _pin) : SV_Target {
	return _pin.Color;
}