#define FBM_THIS(OUT, OCT, LUCAN, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(LUCAN, N); OUT += FUNC; }


float getDensity(float3 p)
{
	//float dist = distance(p, Eye.xyz);
	//float dist = 0.0f;
	
	float d = -p.y;
	int N;
	float s = 0.0f;
	
	d += noise3d(p* 0.01f) * 100.0f;
	
	FBM_THIS(s, 1.0f, 2.06f, noise3d(p * 0.006f ));
	d = s ;
	

	//float detail = max(16.0f - pow(dist, 0.34f), 2.0f); //15.* .. 2
	//FBM_THIS(s, detail, 2.05f, noise3d(p * 0.006f * float3(SCALE, SCALE * (0.5f - 0.5f / SCALE), SCALE)) / SCALE);
	
	//float n = noise3d(p *0.006f *MIN_HEIGHT);
	//d += sin(p.x *0.01f *n) * abs(noise3d(p * 0.01f * MIN_HEIGHT)) * 80.0f; 
	//d += sin(noise3d(p*0.1f)); // bit of texture
	//d += s;
	return d;
}

const static float3 FOG_COLOR = float3(0.9f, 0.9f, 0.9f);
float4 getFog(float3 p, float dist)
{
	return 0.0f;
}

/*float3 getPointFog(float3 rgb, float dist, float3 d)
{
   float fogAmount = c * exp(-Eye.y*b) * (1.0-exp( -dist*d.y*b ))/d.y;
   float3  fogColor  = vec3(0.5f,0.6f,0.7f);
   return lerp( rgb, fogColor, fogAmount );
}*/