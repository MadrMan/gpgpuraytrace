
const static float SHADOW_LENGTH = 100.0f;
const static float4 SHADOW_COLOR = float4(0.08f, 0.12f, 0.14f, 0.6f);
const static float SHADOW_MIP_BIAS = 3.2f;

float3 getColor(float3 p, float3 n, float3 d, float dist)
{
	float4 color = float4(0.9, 0.75f, 0.35f, 0.1f);
	
	//Shading
	float brightness = dot(n, SunDirection);
	float mipf = max(0.5f * log2(dist), 0);

	//Calculate shadow
	/*float precision = max((mipf - SHADOW_MIP_BIAS) * 3.0f, 1.0f) * 8.0f;
	RayResult rr = traceRay(p, 0.4f, SHADOW_LENGTH, precision, SunDirection, true, true);

	if(rr.density > 0.0f)
	{
		//If there is shadow
		brightness *= 0.1f;
	} else {
		//If the sun can reach here OR full mist was hit
		brightness = saturate(brightness - rr.fcolord.w);
		//return float3(1.0f, 0.0f, 0.0f); //rr.fcolord.w;
	}*/
	
	//Specular
	float fresnel = dot(-d, n);
	float specular = saturate(pow(max(fresnel, 0.0f), 40.0f)) * color.a;
	color.rgb += specular;

	//Dim color based on brightness of point
	color *= lerp(SHADOW_COLOR, 1, brightness);
	
	return color.rgb;
}