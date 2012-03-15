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

#include "noise.hlsl"
#include "sky.hlsl"

//const static float3 FOG_COLOR = float3(0.7f, 0.7f, 0.7f);
const static float3 FOG_COLOR = float3(0.9f, 0.9f, 0.9f);
float4 getFog(float3 p)
{
	/*float d = (noise3d(p * 0.003f) * noise3d(p * 0.009f)) * 6.8f + 0.4f;
	d *= 0.005f;
	d -= abs(p.y - 250.0f) * 0.0002f;
	d = saturate(d);
	float fogd = (noise3d(p * 0.008f) + noise3d(p * 0.015f)) * 0.3f + 1.0f;
	return float4(FOG_COLOR * fogd * d, d);*/
	
	float d = (noise3d(p * 0.004f) * noise3d(p * 0.016f)) * 0.006f + .01f;
	d -= (p.y - 6.0f) * 0.001f;
	d = saturate(d);
	float fogd = (noise3d(p * 0.081f) + noise3d(p * 0.052f)) * 0.8f + 1.0f;
	return float4(FOG_COLOR * fogd * d, d);
}

float getDensity(float3 p)
{
	//float r = length(p - Eye.xyz);

	float d = 0.0f;

	/*p += float3(
		noise3d(p * 0.00682),
		noise3d(p * 0.00474),
		noise3d(p * 0.00641)) * 16.0f;*/
	p *= 0.1f;
	d += -p.y;
	d += noise3d(p * 0.124f) * 8.362f;
	d += noise3d(p * 0.237f) * 4.162f;
	d += noise3d(p * 0.52f) * 2.062f;
	d += noise3d(p * 1.013) * 1.04;
	d += noise3d(p * 2.01f) * 0.5f;
	d += noise3d(p * 4.13f) * 0.25f;
	d += noise3d(p * 8.04f) * 0.125f;
	d += noise3d(p * 16.018f) * 0.063f;
	/*d += noise3d(p * 2.213f) * 1.16f;
	d += noise3d(p * 1.278f) * 3.12f;
	d += noise3d(p * 0.645f) * 6.21f;
	d += noise3d(p * 0.246f) * 14.0f;*/
	//d += pow((abs(noise3d(p * 0.047f)) + 0.6f) * 6.0f, 2.0f) * noise3d(p * 0.02475f);
	d += pow((1.0f - saturate(abs(-1.0 - p.y) * 0.3f)) * 2.0f, 2.3f);
	//d -= saturate(-p.y) * 4.0f;
	return d;
}

struct RayResult
{
	float4 pd; //position : 3, depth : 1
	float4 fcolord; //fog color : 3, fog density : 1
	float density;
};

const static float RAY_STEP = 0.03f;
//const static uint RAY_STEPS = 500000;
const static float MAX_DIST = 100.0f;
const static float RAY_STEP_FACTOR = 1.014f;

RayResult traceRay(float3 p, float dist, float stepmod, float3 dir, bool calcfog)
{
	RayResult rr;
	float4 f = 0.0;
	
	float d = 0.0f;
	float step = RAY_STEP * stepmod - dist * (1.0f - RAY_STEP_FACTOR);

	float4 middleFog = getFog(p + dir * dist * 0.5f);
	f += middleFog * dist;
	
	float3 rayp;
	[loop] while(dist < EndDistance && (step > RAY_STEP * 0.2f))
	{
		rayp = p + dir * dist;
		
		float4 fogstep = 0;

		d = getDensity(rayp);
		if(calcfog) fogstep = getFog(rayp) * step;

		if(f.a > 1.0f || d > 0.0f) 
		{
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

const static float3 ShadowColor = float3(0.1f, 0.1f, 0.13f);
float3 getColor(float3 p, float3 n)
{
	float3 color = 0.0f;

	//Calculate landscape color
	float h = p.y;
	h += 100;
	color = float3(h * 0.01f, h * 0.010f, (h - 80.0f) * 0.03);
	float brightness = saturate(dot(n, SunDirection));
	color *= brightness;
	
	//Cliffs
	//float3 cliffColor = float3(0.5f, 0.3f, 0.1f);
	//cliffColor *= noise3d(p * float3(1.0f, 0.1f, 1.0f)) * 0.5f + 0.5f;
	//color = lerp(cliffColor, color , saturate(abs(n.y) * 1.5f));
	
	//Calculate shadow
	RayResult rr = traceRay(p, 0.1f, 3.0f, SunDirection, true);
	
	if(rr.density > 0.0f)
	{
		//color *= lerp( ShadowColor, rr.fcolor, rr.f);
		color *= ShadowColor;
	} else {
		color *= lerp(color, ShadowColor, rr.fcolord.w * 0.8f);
	}
	
	return color;
}

float3 getNormal(float4 pd)
{
	float2 normalDistance = float2(length(pd.xyz - Eye.xyz) * 0.005f, 0.0f);
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
	pd.dir = normalize(pd.p - Eye.xyz);
	return pd;
}