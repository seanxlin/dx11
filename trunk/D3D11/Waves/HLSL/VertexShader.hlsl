cbuffer cbPerFrame
{
	float4x4 gWorldViewProj; 
};

struct VertexShaderIn
{
	float3 PositionL : POSITION;
    float4 Color : COLOR;
};

struct VertexShaderOut
{
	float4 PositionH : SV_POSITION;
    float4 Color : COLOR;
};

VertexShaderOut main(VertexShaderIn input)
{
    VertexShaderOut output;
	
	// Transform to homogeneous clip space.
	output.PositionH = mul(float4(input.PositionL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    output.Color = input.Color;

	return output;
}