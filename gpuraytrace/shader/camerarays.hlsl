#include "tracing.hlsl"

const static uint TOTAL_SIZE_X = 32;
const static uint TOTAL_SIZE_Y = 32;

const static float CAMERA_NEAR = 0.05f;
const static float CAMERA_FAR = 8000.0f;

RWStructuredBuffer<float4> CameraResults;

struct SBFrameData
{
	uint MinHitDistance;
	uint MaxHitDistance;
};
globallycoherent RWStructuredBuffer<SBFrameData> FrameData;

[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, GROUP_SIZE_Z)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	uint2 pixelScaled = (uint2)((DTid.xy / float2(TOTAL_SIZE_X, TOTAL_SIZE_Y)) * ScreenSize);
	PixelData pd = getPixelRay(pixelScaled);
	
	float3 color = 0.0f;
	RayResult rr = traceRay(pd.p, CAMERA_NEAR, CAMERA_FAR, 1.0f, pd.dir, false, true);
	if(rr.density < 0.0f) 
	{
		rr.pd.w = 0.0f; //Set sky to distance 0
	} else {
		//Hit something
		InterlockedMax(FrameData[0].MaxHitDistance, asuint(rr.pd.w));
		InterlockedMin(FrameData[0].MinHitDistance, asuint(rr.pd.w));
	}
	CameraResults[DTid.y * TOTAL_SIZE_X + DTid.x] = rr.pd;
}