#include "tracing.hlsl"
#include "sky.hlsl"
#include "color.hlsl"

//float3 h2r(float h,float s,float v){return lerp(saturate((abs(frac(h+float3(1,2,3)/3)*6-3)-1)),1,s)*v;}

cbuffer PerDispatch
{
	uint2 ThreadOffset;
};

StructuredBuffer<float2> CellDistance;
RWTexture2D<float4> texOut;

float3 traceSample(PixelData pd, float3 pdn, float2 plane)
{
	float3 color = 0.0f;

	RayResult rr = traceRay(pd.p, plane.x, plane.y, 1.0f, pd.dir, true, false);

	float skyAmount = rr.pd.w  * 0.0005f;
	skyAmount = saturate(skyAmount * skyAmount);

	SkyColor scat = getRayleighMieColor(pdn);
	float3 spaceColor = getSpaceColor(pdn);
	float3 skyColor = scat.mie + scat.rayleigh + spaceColor;
	
	if(rr.density > 0.0f) //we've hit something
	{
		float3 n = getNormal(float4(rr.pd.xyz, rr.density));
		color = getColor(rr.pd.xyz, n, pdn, rr.pd.w);
		color = lerp(color, rr.fcolord.xyz, rr.fcolord.w);
		
		color = lerp(color, scat.rayleigh, skyAmount);
	} else { //we've hit nothing
		color = lerp(skyColor, rr.fcolord.xyz, rr.fcolord.w);
		color = lerp(color, skyColor, skyAmount);
	}

	//float3 stepColor = saturate(float3(rr.steps * 0.004f, rr.steps * 0.1f, rr.steps * 0.02f));
	//stepColor.g -= stepColor.b;
	//stepColor.b -= stepColor.r;
	//texOut[DTid.xy] = float4(saturate(color * 0.1f + stepColor), 1.0f);
	//texOut[DTid.xy] =  float4((color + plane.xyx * 0.0001f) * 0.5f, 1.0f);
	
	return color;
}

#if RECORDING
const static float AA_SAMPLES = 2.0f;
#else
const static float AA_SAMPLES = 1.0f;
#endif

[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	uint2 pixelUint = DTid.xy + ThreadOffset;
	float2 pixel = (float2)pixelUint;

	float2 halfTileSize = (ScreenSize / CAMERA_SIZE) * 0.5f;
	float2 screenPosF = (pixel / ScreenSize.xy);
	uint cellPos = floor(screenPosF.y * CAMERA_SIZE.y) * CAMERA_SIZE.x + floor(screenPosF.x * CAMERA_SIZE.x);

	//float2 plane = cellPos % 2 ? CellDistance[cellPos / 2].zw : CellDistance[cellPos / 2].xy;
	float2 plane = float2(CellDistance[cellPos].x, CAMERA_FAR);
	
	float3 color = 0.0f;

	const static float samples = 1.0f; //total = samples ^ 2
	const static float sampleStep = 1.0f / AA_SAMPLES;
	float2 startPosition = -0.5f;
	
	for(float x = 1.0f; x <= AA_SAMPLES; x++)
	{
		for(float y = 1.0f; y <= AA_SAMPLES; y++)
		{
			float2 offset = startPosition + float2(x, y) * sampleStep;
			PixelData pd = getPixelRay(pixel + offset);
			float3 pdn = normalize(pd.dir);
			color += traceSample(pd, pdn, plane);
		}
	}

	color *= rcp(AA_SAMPLES * AA_SAMPLES);
	texOut[pixelUint] = float4(color, 1.0f);
}