#include "tracing.hlsl"

const static float CAMERA_NEAR = 0.05f;
const static float CAMERA_FAR = 5000.0f;

RWStructuredBuffer<float4> CameraResults;

/*struct SBFrameData
{
	uint MinHitDistance;
	uint MaxHitDistance;
};*/
//RWStructuredBuffer<SBFrameData> FrameData;

[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, GROUP_SIZE_Z)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	uint2 pixelScaled = (uint2)((DTid.xy / float2(CAMERA_SIZE_X, CAMERA_SIZE_Y)) * ScreenSize);
	PixelData pd = getPixelRay(pixelScaled);
	
	RayResult rr = traceRay(pd.p, CAMERA_NEAR, CAMERA_FAR, 2.0f, pd.dir, false, true);
	if(rr.density < 0.0f) rr.pd.w = CAMERA_FAR; //Set sky to distance max distance
	CameraResults[DTid.y * CAMERA_SIZE_X + DTid.x] = rr.pd;
}