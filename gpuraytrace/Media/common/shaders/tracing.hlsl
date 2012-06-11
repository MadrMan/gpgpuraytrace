const static uint2 CAMERA_SIZE = uint2(32, 32);

const static float CAMERA_NEAR = 0.05f;
const static float CAMERA_FAR = 5000.0f;

cbuffer XTweakable
{
	float3 SunDirection;
}

cbuffer CBFrame
{
	float4 Eye;
	float4x4 ViewInverse;
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
	float steps;
};

const static float RAY_STEP = 0.03f;
const static float RAY_FINAL_PRECISION = 0.02f;
const static float RAY_MIN_LIMIT = RAY_FINAL_PRECISION * RAY_STEP;
#if RECORDING
const static float RAY_DENSITY_FACTOR = 0.15f;
const static float RAY_STEP_FACTOR = 1.001f;
#else
const static float RAY_DENSITY_FACTOR = 0.35f;
const static float RAY_STEP_FACTOR = 1.009f;
#endif
RayResult traceRay(float3 p, float dist, float enddist, float stepmod, float3 dir, bool calcfog, bool skiprefine);

#include "noise.hlsl"
#include "terrain.hlsl"

RayResult traceRay(float3 p, float dist, float enddist, float stepmod, float3 dir, bool calcfog, bool skiprefine)
{
	RayResult rr;
	float4 f = 0.0;
	float d = 0.0f;
	float totalSteps = 0.0f;
	float dirLength = length(dir);
	float step = RAY_STEP * stepmod * dirLength - dist * (1.0f - RAY_STEP_FACTOR); 
	float lastStep = step;
	dir /= dirLength;
	
	//step = max(0.01f , step);
	if(calcfog) 
	{
		float4 middleFog = getFog(p + dir * dist * 0.5f, dist * 0.5f);
		f += middleFog * dist;
	}
	
	float3 rayp;
	 // * step; //-1.0f;
	//bool refining = false;
	[loop] while(dist < enddist && (step > RAY_MIN_LIMIT))
	{
		totalSteps++;
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
			//refining = true;
			dist -= lastStep;
			step *= 0.3f;
			//lastStep = step;
			f -= fogstep;
		} else {
			float stepmult = 1.0f + pow(abs(min(d + 5.0f, 0.0f)), RAY_DENSITY_FACTOR);
			
			step *= RAY_STEP_FACTOR;
			lastStep = step * stepmult;
			dist += lastStep;
			f += fogstep;
			//if(!refining) step = RAY_STEP * stepmod * dirLength - dist * (1.0f - RAY_STEP_FACTOR); 
		}
	}
	
	rr.pd = float4(rayp, dist);
	rr.fcolord = f;
	rr.density = d;
	rr.steps = totalSteps;
	return rr;
}

float3 getNormal(float4 pd)
{
	float dist = distance(pd.xyz, Eye.xyz);
	float2 normalDistance = float2(dist * 0.005f, 0.0f);
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

PixelData getPixelRay(float2 pixel)
{
	float4 screenLocation = float4(((pixel + 0.5f) / ScreenSize.xy - 0.5f) * 2.0f, 1.0f, 1.0f);
	screenLocation.x *= Projection._22;
	screenLocation.y *= Projection._11;

	PixelData pd;
	pd.p = mul(screenLocation, ViewInverse).xyz;
	pd.dir = pd.p - Eye.xyz;
	return pd;
}