cbuffer PerFrame
{
	float4 Eye;
	float4x4 ViewInverse;
	float4x4 Projection;
}

const static float2 ScreenSize = float2(800.0f, 600.0f);

const static float RAY_DISTANCE = 10.0f;
const static float RAY_STEP = 0.04f;
const static int RAY_STEPS = 1000;

RWTexture2D<float4> texOut : register(u0);

[numthreads(10, 10, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float3 result = float3(0.0f, 0.0f, 0.0f);

	float4 screenLocation = float4((DTid.xy / ScreenSize.xy - 0.5f) * 2.0f, 1.0f, 1.0f);
	screenLocation.x /= Projection._m11;
	screenLocation.y /= Projection._m22;
	
	float3 rayStart = mul(screenLocation, ViewInverse).xyz;
	float3 rayDirection = rayStart - Eye.xyz;
	
	float density = 0.0f;
	for(int d = 0; d < RAY_STEPS; d++)
	{
		float3 rayPosition = rayStart + rayDirection * RAY_STEP * d;
		
		float3 blobpos = float3(0.0f, 0.0f, 0.0f);
		density += 1.0f - saturate(length(blobpos - rayPosition));
	}
	
	result = density.xxx * float3(0.01f, 0.1f, 1.0f);
	texOut[DTid.xy] = float4(result, 0.0f);
}