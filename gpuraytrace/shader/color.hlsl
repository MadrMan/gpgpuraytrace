Texture2D<float4> texColor1 : register(t1);
Texture2D<float4> texColor2 : register(t2);
Texture2D<float4> texColor3 : register(t3);
Texture2D<float4> texColor4 : register(t4);
Texture2D<float4> texColor5 : register(t5);
Texture2D<float4> texColor6 : register(t6);

const static float SHADOW_LENGTH = 100.0f;

const static float4 ShadowColor = float4(0.08f, 0.12f, 0.14f, 0.6f);
float3 getColor(float3 p, float3 n, float3 d, float dist)
{
	//Blend normal-based colors
	float3 blend = abs(n) - 0.2f;
	blend *= 7.0f;
	blend = pow(blend, 3.0f);
	blend = max(0, blend);
	blend /= dot(blend, 1);

	float blendCliff1 = saturate(0.5f + noise3d(p * 0.0074f) * 30.5f);
	//float blendCliff2 = saturate(0.5f + noise3d(p * 0.002f + 10.0f) * 70.2f);
	
	uint mip = floor(max(0.5f * log2(dist), 0));
	
	float4 sampleCliff1 =
		texColor1.SampleLevel(state, p.zy * 0.1f, mip) * blend.x + 
		texColor1.SampleLevel(state, p.xy * 0.1f, mip) * blend.z;
	float4 sampleCliff2 =
		texColor2.SampleLevel(state, p.zy * 0.1f, mip) * blend.x + 
		texColor2.SampleLevel(state, p.xy * 0.1f, mip) * blend.z;
	/*float4 sampleCliff3 =
		texColor5.SampleLevel(state, p.zy * 0.1f, 0) * blend.x + 
		texColor5.SampleLevel(state, p.xy * 0.1f, 0) * blend.z;
	float4 sampleCliff4 =
		texColor6.SampleLevel(state, p.zy * 0.1f, 0) * blend.x + 
		texColor6.SampleLevel(state, p.xy * 0.1f, 0) * blend.z;	*/
	/*float4 cliffColor = lerp(
		lerp(sampleCliff1, sampleCliff2, blendCliff1),
		lerp(sampleCliff3, sampleCliff4, blendCliff1),
		blendCliff2);*/
	float4 cliffColor = lerp(sampleCliff1, sampleCliff2, blendCliff1);
		
	float blendFloor = saturate((p.y - 10.0f) + noise3d(p * 0.0532f) * 15.2f);
	float4 sampleRock = texColor3.SampleLevel(state, p.xz * 0.1f, mip);
	float4 sampleGrass = texColor4.SampleLevel(state, p.xz * 0.1f, mip);
	float4 sampleTop = lerp(sampleGrass, sampleRock, blendFloor);
	
	float4 color = cliffColor + sampleTop * blend.y;
	
	//Shading
	float brightness = dot(n, SunDirection);

	//Calculate shadow
	RayResult rr = traceRay(p, 0.1f, SHADOW_LENGTH, 3.0f, SunDirection, true);

	if(rr.density > 0.0f)
	{
		//If there is shadow
		brightness *= 0.1f;
	} else {
		//If the sun can reach here OR full mist was hit
		brightness = saturate(brightness - rr.fcolord.w);
		//return float3(1.0f, 0.0f, 0.0f); //rr.fcolord.w;
	}
	
	//Specular
	float fresnel = dot(-d, n);
	float specular = pow(max(fresnel, 0.0f), 40.0f) * color.a;
	color.rgb += specular;

	//Dim color based on brightness of point
	color *= lerp(ShadowColor, 1, brightness);
	
	return color.rgb;
}