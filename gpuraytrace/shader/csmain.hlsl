#include "noise.hlsl"

/*cbuffer CBTweakable
{
	float3 SunDirection;
}*/

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
const static float3 FOG_COLOR = normalize(float3(0.5f, 0.6f, 0.7f));
float4 getFog(float3 p)
{
	float d = noise3d(p * 0.05f) * 2.0f + 1.0f;		// pluimen
	d *= 0.005f;
	d -= p.y * 0.0001f;
	d = saturate(d);
	float fogd = (noise3d(p * 0.006f) + noise3d(p * 0.1f)) * 2.8f + 1.0f;
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

const static float RAY_STEP = 0.1f;
const static uint RAY_STEPS = 50000;
const static float MAX_DIST = RAY_STEP * RAY_STEPS;
float2 traceRay(float3 p, float stepmod, float3 dir, out float3 fogcol, out float3 pout)
{
	float4 f = 0.0;
	
	float step = RAY_STEP * stepmod;
	float s = 0.5f;
	float d = 0.0f;
	float3 rayp;
	[loop] while(s < MAX_DIST && step > RAY_STEP * 0.1f)
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
			step *= 1.015f;
			s += step;
			f += fogstep;
		}
	}
	
	pout = rayp;
	fogcol = f.rgb;
	return float2(d, f.a);
}

const static float3 SunDirection = float3(0.4f, 0.4f, 0.4f);
const static float3 ShadowColor = float3(0.1f, 0.1f, 0.2f);
float3 getColor(float3 p, float3 n)
{
	float3 color = 0.0f;
	
	//Calculate landscape color
	float h = p.y;
	h += 100;
	color = float3(h * 0.01f, h * 0.010f, (h - 80.0f) * 0.03);
	float brightness = saturate(dot(n, normalize(SunDirection)));
	color *= brightness;
	
	//Calculate shadow
	float3 fog;
	float3 pout;
	float2 d = traceRay(p, 4.0f, SunDirection, fog, pout);
	
	if(d.x > 0.0f)
	{
		color *= lerp(ShadowColor, fog, d.y);
	} else {
		color = lerp(color, fog, d.y);
	}
	

	return color;
}

float3 getSky(float3 dir)
{
	float3 c = float3(0.0f, 0.0f, 1.0f - dir.y * 0.6f);
	c.rg += (c.b - 0.6f) * 1.0f;
	
	float hemi = saturate(dot(dir, normalize(SunDirection)));
	c.rgb += pow(hemi, 14.0f);
	return c;
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

RWTexture2D<float4> texOut : register(u0);
[numthreads(20, 20, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	PixelData pd = getPixelRay(DTid.xy);
	
	float3 p;
	float3 color;
	float2 d = traceRay(pd.p, 1.0f, pd.dir, color, p);
	
	if(d.x > 0.0f) 							//-- we've hit something
	{
		float3 n = getNormal(p, d.x);
		float3 tcolor = getColor(p, n);
		color = lerp(tcolor, color, d.y);
	} else { 								// -- we've hit nothing
		float3 scolor = getSky(pd.dir);
		color = lerp(scolor, color, d.y);
	}
	
	texOut[DTid.xy] = float4(color, 0.0f);
}