#include "tracing.hlsl"
#include "sky.hlsl"

//float3 h2r(float h,float s,float v){return lerp(saturate((abs(frac(h+float3(1,2,3)/3)*6-3)-1)),1,s)*v;}

struct SBFrameData
{
	uint MinHitDistance;
	uint MaxHitDistance;
};
globallycoherent RWStructuredBuffer<SBFrameData> FrameData;

RWTexture2D<float4> texOut : register(u0);
[numthreads(16, 16, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	PixelData pd = getPixelRay(DTid.xy);
	
	float3 color = 0.0f;
	RayResult rr = traceRay(pd.p, StartDistance, EndDistance, 1.0f, pd.dir, true);

	if(rr.density > 0.0f) //we've hit something
	{
		float3 n = getNormal(float4(rr.pd.xyz, rr.density));
		float3 tcolor = getColor(rr.pd.xyz, n);
		color = lerp(tcolor, rr.fcolord.xyz, rr.fcolord.w);
		
		InterlockedMax(FrameData[0].MaxHitDistance, asuint(rr.pd.w));
		InterlockedMin(FrameData[0].MinHitDistance, asuint(rr.pd.w));
	} else { //we've hit nothing
		float3 scolor = 0;
		if(pd.dir.y > -0.5f) scolor = getSky(pd.dir);	
		color = lerp(scolor, rr.fcolord.xyz, rr.fcolord.w);
	}
	texOut[DTid.xy] = float4(color, 1.0f);
}