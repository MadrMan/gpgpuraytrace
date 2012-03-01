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

float getDensity(float3 position)
{
	float d = 0.0f;

	const static float p = 0.45f;
	float3 scaled = position * 0.1f;
	
	//d = noise3d(scaled) - position.y * 0.1f;
	
	for(uint x = 0; x < 2; x++)
	{
		d += noise3d(scaled * pow(2, x)) * pow(p, x);
	}
	
	//d += sin(position.y) * 0.4f;
	d -= position.y * 0.15f;
	
	return d;

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
	float d = getDensity(position);
	
	if(d < 0.0f) return float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	float2 normalDistance = float2(length(position - Eye.xyz) * 0.005f, 0.0f);
	float3 normal = normalize(float3(
		 getDensity(position - normalDistance.xyy) - d,
		 getDensity(position - normalDistance.yxy) - d,
		 getDensity(position - normalDistance.yyx) - d
	));
	
	//return float4(normal * 0.5f + 0.5f, 1.0f);
	
	float h = position.y;
	h += 100;
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
	float3 rayPosition;
	
	//[loop] while(d < maxDist && step > (d * 0.001f))
	[loop] while(d < maxDist && step > RAY_STEP * 0.1f)
	{
		rayPosition = rayStart + rayDirection * d;
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
	}else{
		/*step = RAY_STEP * 5.0f;
		d = 0.5f;
		rayStart = rayPosition;
		
		[loop] while(d < maxDist * 0.05f && result.a >= 0.1f)
		{
			rayPosition = rayStart + SunDirection * d;
			float4 color = getPoint(rayPosition);

			if(color.a >= 0.1)
			{
				result *= float4(0.1f,0.1f,0.2f,0.0f);
				//break;
			} else {
				step *= 1.05f;
				d += step;
			}
		}*/
	}
	
	texOut[DTid.xy] = result;
}