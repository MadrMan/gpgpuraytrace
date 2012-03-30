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

	float skyAmount = saturate(sqrt(max(rr.pd.w - 50.0f, 0.0f)) * 0.05f);
	
	SkyColor scat = getRayleighMieColor(pd.dir);
	float3 spaceColor = getSpaceColor(pd.dir);
	float3 skyColor = scat.mie + scat.rayleigh + spaceColor;	
	
	if(rr.density > 0.0f) //we've hit something
	{
		float3 n = getNormal(float4(rr.pd.xyz, rr.density));
		color = getColor(rr.pd.xyz, n, pd.dir, rr.pd.w);
		color = lerp(color, rr.fcolord.xyz, rr.fcolord.w);
		
		color = lerp(color, scat.rayleigh, skyAmount);
	} else { //we've hit nothing
		float3 scolor = 0;
		scolor = skyColor;
		color = lerp(scolor, rr.fcolord.xyz, rr.fcolord.w);
		
		color = lerp(color, skyColor, skyAmount);
	}
	texOut[DTid.xy] = float4(color, 1.0f);
}