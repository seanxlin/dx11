struct VertexShaderInput
{
    float3 mPositionW : POSITION;
    float2 mSizeW : SIZE;
};

struct VertexShaderOutput
{
    float3 mCenterW : POSITION;
    float2 mSizeW : SIZE;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.mCenterW = input.mPositionW;
    output.mSizeW = input.mSizeW;

	return output;
}