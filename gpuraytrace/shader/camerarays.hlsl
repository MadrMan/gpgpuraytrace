#include "tracing.hlsl"

const static uint DISPATCH_SIZE = 1;
const static uint GROUP_SIZE = 32;
const static uint TOTAL_SIZE = GROUP_SIZE * DISPATCH_SIZE;
const static float CAMERA_DISTANCE = 1000.0f;

RWStructuredBuffer<float4> CameraResults;

[numthreads(GROUP_SIZE, GROUP_SIZE, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	uint2 pixelScaled = (uint2)((DTid.xy / float2(TOTAL_SIZE, TOTAL_SIZE)) * ScreenSize);
	PixelData pd = getPixelRay(pixelScaled);
	
	float3 color = 0.0f;
	RayResult rr = traceRay(pd.p, StartDistance, CAMERA_DISTANCE, 1.0f, pd.dir, false);
	if(rr.density < 0.0f) rr.pd = 0; //Set sky to 0
	CameraResults[DTid.y * TOTAL_SIZE + DTid.x] = rr.pd;
}