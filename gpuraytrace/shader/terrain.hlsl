#define FBM_THIS(OUT, OCT, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(2.0f, N); OUT += FUNC; }

float getDensity(float3 p)
{
	float d = 0.0f;
	
	d += -p.y;	
	float N;	
	
	float g = noise3d(p * 0.01f);
	float3 noh = float3(1.0f, abs(g) * 1.4f + 0.1f, 1.0f);
	
	//d += abs(noise3d(p * 0.83f)) * 0.2f;
	//d += abs(noise3d(p * 0.48f)) * 0.46f;
	FBM_THIS(d, 7.0f, abs(noise3d(p * float3(0.011f, 0.0013f, 0.011f) * SCALE + g * 0.32f)) * 210.0f * g / SCALE);
	//d += abs(noise3d(pg * float3(0.021f, 0.0013f, 0.021f))) * 100.0f * g;
	d -= 50.0f;
	
	FBM_THIS(d, 5.0f, -((noise3d(p * 0.002f * noh * SCALE) + 0.1f) * 0.5f + 0.5f) * (220.0f / SCALE));
	//d += sqrt(abs(noise3d(p * 0.02f * noh)) + 0.1f) * 120.0f;
	
	
	//d += pow((abs(noise3d(p * 0.047f)) + 0.6f) * 6.0f, 2.0f) * noise3d(p * 0.02475f);
	//d += pow((1.0f - saturate(abs(-1.0 - p.y) * 0.3f)) * 2.0f, 2.3f);
	//d -= saturate(-p.y) * 4.0f;
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