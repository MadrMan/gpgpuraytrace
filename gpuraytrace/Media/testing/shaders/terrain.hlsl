#define FBM_THIS(OUT, OCT, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(2.0f, N); OUT += FUNC; }

float getDensity(float3 p)
{
	float d = 0.0f;
	
	d += -p.y;	
	
	d += sin(p.x * 0.1f) * sin(p.z * 0.2f);
	d += sin(p.z * 0.3f) * sin(p.x * 0.4f);
	
	return d;
}

float4 getFog(float3 p, float dist)
{
	float fogd = 0.0f;
	float d = 0.0001f;
	
	dist = saturate(1.0f - dist * 0.012f);
	//float falloff = saturate(1.0f - (dist * dist * 0.1f));
	//if(falloff > 0.0f)
	//{
		//float d = (noise3d(p * 0.004f) * noise3d(p * 0.016f)) * 0.006f + .01f;
		//float d = 0.010f;
		d -= (p.y - 6.0f) * 0.00003f;
		d = saturate(d);
		//float fogd = (abs(noise3d(p * 0.046f)) + abs(noise3d(p * 0.021f) * 8.0f)) * 0.4f + 0.1f;
		fogd = abs(noise3d(p * 0.1261f)) * 0.2f + 0.8f;
	//}
	return float4(FOG_COLOR * fogd * d, d) * dist;
}

/*float3 getPointFog(float3 rgb, float dist, float3 d)
{
   float fogAmount = c * exp(-Eye.y*b) * (1.0-exp( -dist*d.y*b ))/d.y;
   float3  fogColor  = vec3(0.5f,0.6f,0.7f);
   return lerp( rgb, fogColor, fogAmount );
}*/