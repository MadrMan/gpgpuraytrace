cbuffer XTweakable
{
	float3 SunDirection;
}

cbuffer CBFrame
{
	float4 Eye;
	float4x4 ViewInverse;
	float StartDistance;
	float EndDistance;
	float Time;
}

cbuffer CBPermanent
{
	float2 ScreenSize;
	float4x4 Projection;
}

struct RayResult
{
	float4 pd; //position : 3, depth : 1
	float4 fcolord; //fog color : 3, fog density : 1
	float density;
};

const static float RAY_STEP = 0.03f;
//const static uint RAY_STEPS = 500000;
const static float RAY_STEP_FACTOR = 1.014f;
const static float RAY_FINAL_PRECISION = RAY_STEP * 0.2f;
RayResult traceRay(float3 p, float dist, float enddist, float stepmod, float3 dir, bool calcfog, bool skiprefine);

#include "noise.hlsl"
#include "terrain.hlsl"

RayResult traceRay(float3 p, float dist, float enddist, float stepmod, float3 dir, bool calcfog, bool skiprefine)
{
	RayResult rr;
	float4 f = 0.0;
	
	float d = 0.0f;
	
	float dirLength = length(dir);
	float step = RAY_STEP * stepmod * dirLength - dist * (1.0f - RAY_STEP_FACTOR);
	dir /= dirLength;
	
	if(calcfog) 
	{
		float4 middleFog = getFog(p + dir * dist * 0.5f, dist * 0.5f);
		f += middleFog * dist;
	}
	
	float3 rayp;
	[loop] while(dist < enddist && (step > RAY_FINAL_PRECISION))
	{
		rayp = p + dir * dist;
		
		float4 fogstep = 0;

		d = getDensity(rayp);
		if(calcfog) 
		{
			fogstep = getFog(rayp, dist) * step;
		}

		if(/*f.a > 1.0f ||*/ d > 0.0f) 
		{
			if(skiprefine) break;
		
			dist -= step;
			step *= 0.4f;
			f -= fogstep;
		} else {
			step *= RAY_STEP_FACTOR;
			dist += step;
			f += fogstep;
		}
	}
	
	rr.pd = float4(rayp, dist);
	rr.fcolord = f;
	rr.density = d;
	return rr;
}

float3 getNormal(float4 pd)
{
	float2 normalDistance = float2(distance(pd.xyz, Eye.xyz) * 0.005f, 0.0f);
	return normalize(float3(
		 getDensity(pd.xyz - normalDistance.xyy) - pd.w,
		 getDensity(pd.xyz - normalDistance.yxy) - pd.w,
		 getDensity(pd.xyz - normalDistance.yyx) - pd.w
	));
}

struct PixelData
{
	float3 p;
	float3 dir;
};

PixelData getPixelRay(uint2 DTid)
{
	float4 screenLocation = float4((DTid / ScreenSize.xy - 0.5f) * 2.0f, 1.0f, 1.0f);
	screenLocation.x /= Projection._m11;
	screenLocation.y /= Projection._m22;

	PixelData pd;
	pd.p = mul(screenLocation, ViewInverse).xyz;
	pd.dir = pd.p - Eye.xyz;
	return pd;
}