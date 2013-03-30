#define NUM_CONTROL_POINTS 16

cbuffer cbPerFrame : register(b0)
{
	float4x4 gWorld;
	float4x4 gWorldViewProjection;
    float4x4 gTexTransform;
    float gInGameTime;
};

struct PatchTess
{
	float mEdgeTess[4] : SV_TessFactor;
	float mInsideTess[2] : SV_InsideTessFactor;
};

struct DomainShaderInput
{
    float3 mPositionL : POSITION;
};

struct DomainShaderOutput
{
	float4 mPositionH : SV_POSITION;
    float3 mPositionW : POSITION;
    float3 mNormalW : NORMAL;
    float2 mTexCoord : TEXCOORD;
};

float4 BernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( invT * invT * invT,
                   3.0f * t * invT * invT,
                   3.0f * t * t * invT,
                   t * t * t );
}

float3 CubicBezierSum(const OutputPatch<DomainShaderInput, NUM_CONTROL_POINTS> bezpatch, float4 basisU, float4 basisV)
{
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    sum  = basisV.x * (basisU.x * bezpatch[0].mPositionL + basisU.y * bezpatch[1].mPositionL + basisU.z * bezpatch[2].mPositionL + basisU.w * bezpatch[3].mPositionL );
    sum += basisV.y * (basisU.x * bezpatch[4].mPositionL + basisU.y * bezpatch[5].mPositionL + basisU.z * bezpatch[6].mPositionL + basisU.w * bezpatch[7].mPositionL );
    sum += basisV.z * (basisU.x * bezpatch[8].mPositionL + basisU.y * bezpatch[9].mPositionL + basisU.z * bezpatch[10].mPositionL + basisU.w * bezpatch[11].mPositionL);
    sum += basisV.w * (basisU.x * bezpatch[12].mPositionL + basisU.y * bezpatch[13].mPositionL + basisU.z * bezpatch[14].mPositionL + basisU.w * bezpatch[15].mPositionL);

    return sum;
}

float4 dBernsteinBasis(float t)
{
    float invT = 1.0f - t;

    return float4( -3 * invT * invT,
                   3 * invT * invT - 6 * t * invT,
                   6 * t * invT - 3 * t * t,
                   3 * t * t );
}

[domain("quad")]
DomainShaderOutput main(PatchTess patchTess, 
             float2 uv : SV_DomainLocation, 
             const OutputPatch<DomainShaderInput, NUM_CONTROL_POINTS> bezPatch)
{
	DomainShaderOutput output;
	
	float4 basisU = BernsteinBasis(uv.x);
	float4 basisV = BernsteinBasis(uv.y);

	float3 p = CubicBezierSum(bezPatch, basisU, basisV);	
	
    float x = clamp(p.x, -5.0f, 5.0f);
    float z = clamp(p.z, -5.0f, 5.0f);
    float yOffset = 0.05f * x * z * sin(gInGameTime);
    p.y *= yOffset;

    // Compute positions and texture coordinates.
    output.mPositionH = mul(float4(p, 1.0f), gWorldViewProjection);
    output.mPositionW = mul(float4(p, 1.0f), gWorld).xyz;
    output.mTexCoord = mul(float4(uv, 0.0f, 1.0f), gTexTransform).xy;

    // Compute normal for the current position
	float4 dBasisU = dBernsteinBasis(uv.x);
	float4 dBasisV = dBernsteinBasis(uv.y);

    float3 dpdu = CubicBezierSum(bezPatch, dBasisU, basisV);
    dpdu.y *= yOffset;

    float3 dpdv = CubicBezierSum(bezPatch, basisU, dBasisV);
    dpdv.y *= yOffset;

    float3 normalW = mul(cross(dpdu, dpdv), gWorld).xyz;
    output.mNormalW = normalize(normalW);

	return output;
}