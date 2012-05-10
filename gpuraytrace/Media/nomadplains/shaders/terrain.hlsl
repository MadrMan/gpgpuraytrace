#define FBM_THIS(OUT, OCT, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(2.0f, N); OUT += FUNC; }

const static float3 NO_HEIGHT = float3(1.0f, 0.0f, 1.0f);
const static float3 MIN_HEIGHT = float3(1.0f, 0.10f, 1.0f);
const static float3 MORE_HEIGHT = float3(1.0f, 2.5f, 1.0f);
float getDensity(float3 p)
{
	float d = -p.y;
	int N;
	
	float s = 0.0f;
	
	float op = p * 0.01f * MORE_HEIGHT;
	float wd = noise3d(op);
	float wd2 = noise3d(op + float3(0.0f, 40.0f, 0.0f));
	float3 wp = p + float3(wd, 0.0f, wd2) * 30.0f;

	FBM_THIS(s, 12, noise3d(wp * 0.003f * SCALE * NO_HEIGHT) / SCALE);
	s *= 30.0f;
	
	/*s += noise3d(wp * 0.003f * NO_HEIGHT) * 30.0f;
	s += noise3d(wp * 0.0062f * NO_HEIGHT) * 20.0f;
	s += noise3d(p * 0.01f * MIN_HEIGHT) * 10.0f;
	s += noise3d(p * 0.03f * MIN_HEIGHT) * 4.0f;
	s += noise3d(p * 0.05f * MIN_HEIGHT) * 2.0f;*/
	
	//float r = 1.0f - abs(noise3d(wp * 0.003f * MIN_HEIGHT));
	//s *= saturate(pow(abs(r), 24.3f) * 10.4f) + 0.5f;
	//s += saturate(noise3d(p * 0.0023f) * 4.0f + 1.0f) * 200.0f;
	
	s = pow(abs(s + 1.0f) * 35.0f, 0.8f);
	
	float detail = 0.0f;
	detail += noise3d(p * 0.3f * MORE_HEIGHT) * 0.08f;
	detail *= abs(noise3d(p * 0.1331f));
	detail += noise3d(p * 0.8f * MORE_HEIGHT) * 0.02f;
	detail += noise3d(p * 0.1133f * MORE_HEIGHT) * 0.23f;
	s += detail * 20.0f;
	
	s -= pow(saturate((p.y - 10.0f) * 0.5f), 2.0f) * 10.0f;
	s -= pow(saturate((p.y - 20.0f) * 0.5f), 2.0f) * 10.0f;
	s -= pow(saturate((p.y - 30.0f) * 0.5f), 2.0f) * 20.0f;
	s -= pow(saturate((p.y - 40.0f) * 0.5f), 2.0f) * 20.0f;
	s -= pow(saturate((p.y - 50.0f) * 0.5f), 2.0f) * 20.0f;
	
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