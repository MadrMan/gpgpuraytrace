#include "noise.hlsl"

cbuffer XTweakable
{
	float3 SunDirection;
}

cbuffer CBFrame
{
	float4 Eye;
	float4x4 ViewInverse;
	float4x4 Projection;
	
	float StartDistance;
	float EndDistance;
}

cbuffer CBPermanent
{
	float2 ScreenSize;
}

struct SBFrameData
{
	uint MinHitDistance;
	uint MaxHitDistance;
};

globallycoherent RWStructuredBuffer<SBFrameData> FrameData;

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
	
	float d = (noise3d(p * 0.003f) * noise3d(p * 0.009f)) * 0.006f + .01f;
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
	float4 pd; //position : 3, density : 1
	float4 fcolord; //fog color : 3, fog density : 1
	float dist;
};

const static float RAY_STEP = 0.03f;
//const static uint RAY_STEPS = 500000;
const static float MAX_DIST = 100.0f;
const static float RAY_STEP_FACTOR = 1.014f;

RayResult traceRay(float3 p, float dist, float stepmod, float3 dir)
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
		
		d = getDensity(rayp);
		float4 fogstep = getFog(rayp) * step;

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
	
	rr.pd = float4(rayp, d);
	rr.fcolord = f;
	rr.dist = dist;
	return rr;
}

const static float3 ShadowColor = float3(0.2f, 0.2f, 0.23f);
float3 getColor(float3 p, float3 n)
{
	float3 color = 0.0f;

	//Calculate landscape color
	float h = p.y;
	h += 100;
	color = float3(h * 0.01f, h * 0.010f, (h - 80.0f) * 0.03);
	float brightness = saturate(dot(n, SunDirection));
	color *= brightness;
	
	//Calculate shadow
	RayResult rr = traceRay(p, 0.1f, 3.0f, SunDirection);
	
	if(rr.pd.w > 0.0f)
	{
		//color *= lerp( ShadowColor, rr.fcolor, rr.f);
		color *= ShadowColor;
	} else {
		color *= lerp(color, ShadowColor, rr.fcolord.w * 0.8f);
	}
	
	return color;
}

float3 getSky(float3 dir)
{
	float3 c = float3(0.0f, 0.0f, 1.0f - dir.y * 0.6f);
	c.rg += (c.b - 0.6f) * 1.0f;
	
	float hemi = saturate(dot(dir, SunDirection));
	return lerp(c.rgb, float3(2.0f, 2.0f, 1.2f), pow(hemi, 14.0f));
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

//float3 h2r(float h,float s,float v){return lerp(saturate((abs(frac(h+float3(1,2,3)/3)*6-3)-1)),1,s)*v;}

RWTexture2D<float4> texOut : register(u0);
[numthreads(16, 16, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	PixelData pd = getPixelRay(DTid.xy);
	
	float3 color = 0.0f;
	RayResult rr = traceRay(pd.p, StartDistance, 1.0f, pd.dir);

	if(rr.pd.w > 0.0f) 							//-- we've hit something
	{
		float3 n = getNormal(rr.pd);
		float3 tcolor = getColor(rr.pd.xyz, n);
		color = lerp(tcolor, rr.fcolord.xyz, rr.fcolord.w);
		
		InterlockedMax(FrameData[0].MaxHitDistance, asuint(rr.dist));
		InterlockedMin(FrameData[0].MinHitDistance, asuint(rr.dist));
	} else { 								// -- we've hit nothing
		float3 scolor = getSky(pd.dir);
		color = lerp(scolor, rr.fcolord.xyz, rr.fcolord.w);
	}

	texOut[DTid.xy] = float4(color, 1.0f);
}