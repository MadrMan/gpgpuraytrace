#define FBM_THIS_C(OUT, OCT, LUCAN, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(LUCAN, N); OUT += FUNC; }

const static float SHADOW_LENGTH = 100.0f;
const static float4 SHADOW_COLOR = float4(0.08f, 0.12f, 0.14f, 0.6f);
const static float SHADOW_MIP_BIAS = 3.2f;

float3 getColor(float3 p, float3 n, float3 d, float dist)
{
	float3 blend = abs(n) - 0.2f;
	blend *= 7.0f;
	blend = pow(blend, 3.0f);
	blend = max(0, blend);
	blend /= dot(blend, 1);
	float blendcliff = blend.x + blend.z;

	float n1 = noise3d(p * 0.2f) * 0.5f + 0.5f;
	
	float s = 0.0f;
	float N;
	FBM_THIS(s, 4, 2.05f, (noise3d(p * 0.02f * SCALE) * 0.5f + 0.5f) / SCALE);
	//FBM_THIS(ssmall, 3, 2.05f, (noise3d(p * 4.6f * SCALE) * 0.5f + 0.5f) / SCALE);
	//s *= pow(ssmall, 4.0f);
	
	float4 color1 = float4(193, 131, 92, 0) / 255.0f;
	float4 color2 = float4(201, 153, 69, 0) / 255.0f;
	
	float h = abs(p.y);
	//color1 -= abs(noise3d(float3(p.y * 0.5f, p.x * 0.01f, p.z * 0.01f))) * 0.3f;
	s = 0.0f;
	FBM_THIS(s, 20, 2.03f, (abs(noise3d(float3(p.y * 0.5f, p.x * 0.01f, p.z * 0.01f) * SCALE)) / SCALE);
	color1 -= s * 0.5f;
	
	s *= 30.0f;
	float4 color = color1; //lerp(color1, color2, saturate(pow(s, 8.0f)));
	
	/*float4 blue = float4(0.0f, 0.0f, 1.0f, 0.0f);
	float4 cliffcol1 = float4(166.0f, 137.0f, 133.0f, 0.0f) / 255.0f;
	float4 cliffcol2 = float4(147.0f, 109.0f, 121.0f, 0.0f) / 255.0f;
	float4 cliffcol = lerp(cliffcol1, cliffcol2, pow(saturate(n1 + 0.4f), 100.0f));
	color = lerp(color, cliffcol, blendcliff);*/
	
	
	

	
	
	
	
	
	
	
	
	color.a = 0.2f;
	
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
	float fresnel = dot(SunDirection, reflect(n, -d));
	float specular = saturate(pow(max(fresnel, 0.0f), 40.0f)) * color.a;
	color.rgb += specular;

	//Dim color based on brightness of point
	color *= lerp(SHADOW_COLOR, 1, brightness);
	
	return color.rgb;
}