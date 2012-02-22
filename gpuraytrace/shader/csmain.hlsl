cbuffer PerFrame
{
	float4x4 View;
	float4x4 Projection;
}

const static float2 ScreenSize = float2(800.0f, 600.0f);

const static float RAY_DISTANCE = 10.0f;
const static float RAY_STEP = 0.01f;
const static int RAY_STEPS = (int)(RAY_DISTANCE / RAY_STEP);

RWTexture2D<float4> texOut : register(u0);

[numthreads(10, 10, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float3 result = float3(0.0f, 0.0f, 0.0f);

	float2 screenLocation = (DTid.xy / ScreenSize.xy - 0.5f) * 2.0f;
	//screenLocation.x /= Projection_m11;
	//screenLocation.y /= Projection_m22;
	
	//float3 cameraPosition = float3(20.0f, 20.0f, 20.0f);
	//float3 rayStart = (screenLocation * inverse(View)).xyz;
	//float3 rayDirection = rayStart - cameraPosition
	
	//for(float d = 
	
	result = screenLocation.xyy * 0.5f + 0.5f;
	texOut[DTid.xy] = float4(result, 0.0f);
}