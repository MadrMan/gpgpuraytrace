#define FBM_THIS(OUT, OCT, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(2.0f, N); OUT += FUNC; }

const static float3 NO_HEIGHT = float3(1.0f, 0.0f, 1.0f);
const static float3 MIN_HEIGHT = float3(1.0f, 0.15f, 1.0f);
float getDensity(float3 p)
{
	float d = -p.y;
	int N;
	
	float s = 0.0f;
	s = noise3d(p * 0.01f * NO_HEIGHT) * 10.0f;
	s += noise3d(p * 0.003f * NO_HEIGHT) * 40.0f;
	s += noise3d(p * 0.03f * MIN_HEIGHT) * 4.0f;
	s += noise3d(p * 0.3f * MIN_HEIGHT) * 0.2f;

	s = pow(abs(s + 2.0f) * 35.0f, 0.8f);
	
	s -= pow(saturate((p.y - 10.0f) * 0.5f), 1.0f) * 10.0f;
	s -= pow(saturate((p.y - 20.0f) * 0.5f), 1.0f) * 10.0f;
	s -= pow(saturate((p.y - 30.0f) * 0.5f), 1.0f) * 10.0f;
	s -= pow(saturate((p.y - 40.0f) * 0.5f), 1.0f) * 20.0f;
	
	s += pow(saturate(-p.y + 20.0f), 2.0f) * 20.0f;
	
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