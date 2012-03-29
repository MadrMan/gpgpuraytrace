#include "tracing.hlsl"
#include "sky.hlsl"
#include "color.hlsl"

//float3 h2r(float h,float s,float v){return lerp(saturate((abs(frac(h+float3(1,2,3)/3)*6-3)-1)),1,s)*v;}

cbuffer PerDispatch
{
	uint2 ThreadOffset;
};

RWTexture2D<float4> texOut : register(u0);
[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	DTid.xy += ThreadOffset;
	PixelData pd = getPixelRay(DTid.xy);
	
	float3 color = 0.0f;
	RayResult rr = traceRay(pd.p, StartDistance, EndDistance, 1.0f, pd.dir, true, false);

	if(rr.density > 0.0f) //we've hit something
	{
		float3 n = getNormal(float4(rr.pd.xyz, rr.density));
		float3 tcolor = getColor(rr.pd.xyz, n, pd.dir, rr.pd.w);
		color = lerp(tcolor, rr.fcolord.xyz, rr.fcolord.w);
	} else { //we've hit nothing
		float3 scolor = 0;
		scolor = getSky(pd.dir);	
		color = lerp(scolor, rr.fcolord.xyz, rr.fcolord.w);
	}
	texOut[DTid.xy] = float4(color, 1.0f);
}