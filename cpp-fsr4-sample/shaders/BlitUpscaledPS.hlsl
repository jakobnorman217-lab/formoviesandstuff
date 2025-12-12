Texture2D inputTex : register(t0);
SamplerState linearSampler : register(s0);

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
    return inputTex.Sample(linearSampler, uv);
}
