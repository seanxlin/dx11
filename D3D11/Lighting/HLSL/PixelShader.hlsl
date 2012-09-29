struct PixelShaderIn
{
	float4 mPositionH : SV_POSITION;
    float4 mColor : COLOR;
};

float4 main(PixelShaderIn input) : SV_TARGET
{
	return input.mColor;
}