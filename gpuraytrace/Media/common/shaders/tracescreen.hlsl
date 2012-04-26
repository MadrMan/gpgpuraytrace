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

[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	DTid.xy += ThreadOffset;
	PixelData pd = getPixelRay(DTid.xy);
	float3 pdn = normalize(pd.dir);
	
	float2 halfTileSize = (ScreenSize / CAMERA_SIZE) * 0.5f;
	float2 pixel = DTid.xy; // - halfTileSize;
	float2 screenPosF = (pixel / ScreenSize.xy);
	uint cellPos = floor(screenPosF.y * CAMERA_SIZE.y) * CAMERA_SIZE.x + floor(screenPosF.x * CAMERA_SIZE.x);

	//float2 plane = cellPos % 2 ? CellDistance[cellPos / 2].zw : CellDistance[cellPos / 2].xy;
	float2 plane = CellDistance[cellPos];
	
	//texOut[DTid.xy] =  plane.xyxy * 0.0001f;
	//return; 
	
	float3 color = 0.0f;
	RayResult rr = traceRay(pd.p, plane.x, plane.y, 1.0f, pd.dir, true, false);

	float skyAmount = rr.pd.w  * 0.0007f;
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

	/*float3 stepColor = saturate(float3(rr.steps * 0.004f, rr.steps * 0.1f, rr.steps * 0.02f));
	stepColor.g -= stepColor.b;
	stepColor.b -= stepColor.r;
	texOut[DTid.xy] = float4(saturate(color * 0.1f + stepColor), 1.0f);*/
	//texOut[DTid.xy] =  float4((color + plane.xyx * 0.0001f) * 0.5f, 1.0f);
	texOut[DTid.xy] = float4(color, 1.0f);
}