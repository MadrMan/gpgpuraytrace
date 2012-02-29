#include "noise.hlsl"
//SamplerState state;
//Texture2D<float> texNoise;

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

const static float RAY_STEP = 0.1f;
const static int RAY_STEPS = 50000;
const static float3 SunDirection = float3(0.4f, 0.4f, 0.4f);

RWTexture2D<float4> texOut : register(u0);

float getHeight(float2 position)
{
	const static float p = 0.45f;
	
	float h = 0.3f;
	float2 scaled = position * 0.01f;
	
	//return sin(position.x * 0.1f) * sin(position.y * 0.1f) * 20.0f;
	for(uint x = 0; x < 6; x++)
	{
		h += noise2d(scaled * pow(2, x)) * pow(p, x);
	}
	
	//float h = sin(position.x * 0.1f) * 0.1f * sin(position.y * 0.1f);
	
	return h * 80.0f;
}

float4 getSky(float3 direction)
{
	float4 c = float4(0.0f, 0.0f, 1.0f - direction.y * 0.6f, 0.0f);
	c.rg += (c.b - 0.6f) * 1.0f;
	
	float hemi = saturate(dot(direction, normalize(SunDirection)));
	c.rgb += pow(hemi, 14.0f);
	return c;
}

//const static float normalDistance = 0.01f;
float4 getPoint(float3 position)
{
	float h = getHeight(position.xz);
	
	if(h < position.y) return float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float normalDistance = length(position - Eye.xyz) * 0.005f;
	
	position.y = h;
	float3 herex = float3(position.x - normalDistance, h, position.z);
	float3 herey = float3(position.x, h, position.z - normalDistance);
	herex.y = getHeight(herex.xz);
	herey.y = getHeight(herey.xz);
	
	position = position.xzy; herex = herex.xzy; herey = herey.xzy;
	float3 normal = normalize(cross(position - herex, position - herey));
	
	//return float4(normal * 0.5f + 0.5f, 1.0f);
	
	h += 45;
	float3 color = float3(h * 0.01f, h * 0.010f, (h - 80.0f) * 0.03);
	//float3 color = float3(fmod(h, 1.0f), 0.0f, 0.0f);
	float brightness = saturate(dot(normal, normalize(SunDirection)));
	return float4(color * brightness, 1.0f);
	//if(h < position.y) return float4(0.0f, 0.0f, 0.0f, 0.0f);
	//return float4(h * 0.05f, h * 0.1f, saturate(-h) + h * 0.03f, 1.0f);
}

[numthreads(20, 20, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 screenLocation = float4((DTid.xy / ScreenSize.xy - 0.5f) * 2.0f, 1.0f, 1.0f);
	screenLocation.x /= Projection._m11;
	screenLocation.y /= Projection._m22;
	
	float3 rayStart = mul(screenLocation, ViewInverse).xyz;
	float3 rayDirection = normalize(rayStart - Eye.xyz);

	float maxDist = RAY_STEP * RAY_STEPS;
	
	float step = RAY_STEP;
	float d = 0.0f;
	
	//[loop] while(d < maxDist && step > (d * 0.001f))
	[loop] while(d < maxDist && step > RAY_STEP * 0.1f)
	{
		float3 rayPosition = rayStart + rayDirection * d;
		float4 color = getPoint(rayPosition);

		if(color.a >= 0.1)
		{
			result = color;
		
			d -= step;
			step *= 0.4f;
		} else {
			step *= 1.015f;
			d += step;
		}
	}
	
	if(result.a < 0.1)
	{
		result = getSky(rayDirection);
	}

	texOut[DTid.xy] = result;
}