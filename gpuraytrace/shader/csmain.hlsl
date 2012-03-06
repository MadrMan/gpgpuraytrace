#include "noise.hlsl"

cbuffer CBTweakable
{
	float3 SunDirection;
}

cbuffer CBFrame
{
	float4 Eye;
	float4x4 ViewInverse;
	float4x4 Projection;
}

cbuffer CBPermanent
{
	float2 ScreenSize;
}

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
	d -= (p.y - 10.0f) * 0.001f;
	d = saturate(d);
	float fogd = (noise3d(p * 0.081f) + noise3d(p * 0.052f)) * 0.8f + 1.0f;
	return float4(FOG_COLOR * fogd * d, d);
}

float getDensity(float3 p)
{
	//float r = length(p - Eye.xyz);

	p *= 0.1f;
	float d = 0.0f;

	d = -p.y;
	d += noise3d(p * 4.03f) * 0.25f;
	d += noise3d(p * 1.96f) * 0.5f;
	d += noise3d(p * 1.01f) * 1.0f;
	d += noise3d(p * 0.53f) * 2.0f;
	d += noise3d(p * 0.21f) * 6.0f;
	d += noise3d(p * 0.09f) * 14.0f;
	d += pow(saturate(-p.y) * 2.0f, 2.3f);
	
	return d;
}

struct RayResult
{
	float d; //density
	float f; //fog
	float3 p; //position
	float3 fcolor; //fog color
};

const static float RAY_STEP = 0.03f;
//const static uint RAY_STEPS = 500000;
const static float MAX_DIST = 5000.0f;

RayResult traceRay(float3 p, float stepmod, float3 dir)
{
	RayResult rr;
	float4 f = 0.0;
	
	float step = RAY_STEP * stepmod;
	float s = 0.5f;
	float d = 0.0f;
	float3 rayp;
	[loop] while(s < MAX_DIST && step > RAY_STEP * 0.2f)
	{
		rayp = p + dir * s;
		
		d = getDensity(rayp);
		float4 fogstep = getFog(rayp) * step;

		if(f.a > 1.0f || d > 0.0f) 
		{
			s -= step;
			step *= 0.4f;
			f -= fogstep;
		} else {
			step *= 1.012f;
			s += step;
			f += fogstep;
		}
	}
	
	rr.p = rayp;
	rr.fcolor = f.rgb;
	rr.d = d;
	rr.f = f.a;
	return rr;
}

const static float3 ShadowColor = float3(0.1f, 0.1f, 0.2f);
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
	RayResult rr = traceRay(p, 4.0f, SunDirection);
	
	if(rr.d > 0.0f)
	{
		//color *= lerp( ShadowColor, rr.fcolor, rr.f);
		color *= ShadowColor;
	} else {
		color *= lerp(color, ShadowColor, rr.f * 0.8f);
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

float3 getNormal(float3 p, float d)
{
	float2 normalDistance = float2(length(p - Eye.xyz) * 0.005f, 0.0f);
	return normalize(float3(
		 getDensity(p - normalDistance.xyy) - d,
		 getDensity(p - normalDistance.yxy) - d,
		 getDensity(p - normalDistance.yyx) - d
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
	RayResult rr = traceRay(pd.p, 1.0f, pd.dir);

	if(rr.d > 0.0f) 							//-- we've hit something
	{
		float3 n = getNormal(rr.p, rr.d);
		float3 tcolor = getColor(rr.p, n);
		color = lerp(tcolor, rr.fcolor, rr.f);
	} else { 								// -- we've hit nothing
		float3 scolor = getSky(pd.dir);
		color = lerp(scolor, rr.fcolor, rr.f);
	}

	texOut[DTid.xy] = float4(color, 1.0f);
}