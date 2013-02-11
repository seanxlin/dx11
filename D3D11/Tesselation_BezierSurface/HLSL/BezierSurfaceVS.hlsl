struct VertexShaderInput
{
	float3 mPositionL : POSITION;
};

struct VertexShaderOutput
{
    float3 mPositionL : POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	
    output.mPositionL = input.mPositionL;
	
	return output;
}