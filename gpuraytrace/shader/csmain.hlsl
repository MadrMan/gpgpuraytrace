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

const static float RAY_STEP = 0.1f;
const static int RAY_STEPS = 100000;

RWTexture2D<float4> texOut : register(u0);

float getHeight(float2 position)
{
	return sin(position.x * 0.1f) * sin(position.y * 0.1f) * 20.0f;
}

float4 getSky(float3 direction)
{
	float4 c = float4(0.0f, 0.0f, 1.0f - direction.y * 0.6f, 0.0f);
	c.rg += (c.b - 0.6f) * 1.0f;
	
	float hemi = saturate(dot(direction, normalize(SunDirection)));
	c.rgb += pow(hemi, 14.0f);
	return c;
}

float4 getPoint(float3 position)
{
	float h = getHeight(position.xz);
	if(h < position.y) return float4(0.0f, 0.0f, 0.0f, 0.0f);
	return float4(h * 0.05f, h * 0.1f, saturate(-h) + h * 0.03f, 1.0f);
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
			step *= 0.25f;
		} else {
			step *= 1.02;
			d += step;
		}
	}
	
	if(result.a < 0.1)
	{
		result = getSky(rayDirection);
	}

	texOut[DTid.xy] = result;
}