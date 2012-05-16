#define FBM_THIS(OUT, OCT, LUCAN, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(LUCAN, N); OUT += FUNC; }

const static float3 NO_HEIGHT = float3(1.0f, 0.0f, 1.0f);
const static float3 MIN_HEIGHT = float3(1.0f, 0.06f, 1.0f);
const static float3 MORE_HEIGHT = float3(1.0f, 2.5f, 1.0f);
const static float3 LOTS_HEIGHT = float3(1.0f, 6.5f, 1.0f);
float getDensity(float3 p)
{
	float dist = distance(p, Eye.xyz);
	//float dist = 0.0f;
	
	float d = -p.y;
	p *= 0.4f;
	
	int N;
	
	float s = 0.0f;
	
float detail = max(16.0f - pow(dist, 0.34f), 2.0f); //15.* .. 2
	
	FBM_THIS(s, detail, 2.05f, noise3d(p * 0.006f * float3(SCALE, SCALE * (0.5f - 0.5f / SCALE), SCALE)) / SCALE);
	
	s *= 30.0f;
	s = pow(abs(s + 1.0f) * 35.0f, 0.8f);
	
	float steep = saturate((noise3d(p.xzz * 0.007138f) - 0.2f) * 6.0f) * 7.5f;
	float floorsize = steep * 1.8f;
	s -= pow(saturate((p.y - 13.0f) * steep), 2.0f) * floorsize;
	s -= pow(saturate((p.y - 16.0f) * steep), 2.0f) * floorsize;
	s -= pow(saturate((p.y - 19.0f) * steep), 2.0f) * floorsize;
	s -= pow(saturate((p.y - 22.0f) * steep), 2.0f) * floorsize;

	s += pow(saturate((-p.y + 10.0f) * 1.6f), 1.5f) * 19.0f;
	
	d += s;
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