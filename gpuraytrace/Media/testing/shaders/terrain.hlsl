#define FBM_THIS(OUT, OCT, FUNC) \
	for(N = 1.0f; N <= OCT; N++) { float SCALE = pow(2.0f, N); OUT += FUNC; }

const static float3 MIN_HEIGHT = float3(1.0f, 0.15f, 1.0f);
	
float getDensity(float3 p)
{
	float d = -p.y;
	
	//flat
	//float n = noise3d(p *0.3f);
	//d += sin(n); 
	
	//sincos
	d +=sin(p.x *0.1f) * cos(p.z*0.1f) *10.0f;
		
	//square block
	//d = -0.0001;
	//d *= sign(p.y);
	//float3 c = abs(p) - 10.0f;
	//d += -min(max(max(c.x, c.y), c.z), 0.0f);
	
	//rocky
	//d += sin(p.x *0.1f) * cos(p.z*0.2f) * 8.0f;// * (noise3d(p *0.3f) *2.0f);
	//d += sin(p.x * 0.3f) * sin(p.z * 0.4f)* 4.0f;
	//d += cos(p.z * 0.5) * sin(p.x * 0.6f) * 2.5f;
	
	//sandy
	//p.y *= 0.1f;
	//float n = noise3d(p *0.006f *MIN_HEIGHT);
	//d += sin(p.x *0.01f *n) * abs(noise3d(p * 0.01f * MIN_HEIGHT)) * 80.0f; 
	//d += sin(noise3d(p*0.1f)); // bit of texture

	//d += abs(sin(noise3d(p*0.1f)));
	return d;
}
const static float3 FOG_COLOR = float3(0.9f, 0.9f, 0.9f);
float4 getFog(float3 p, float dist)
{

	float fogd = 0.0f;
	float d = 0.0f;
	
	dist = saturate(1.0f - dist * 0.0012f);
	float falloff = saturate(1.0f - (dist * dist * 0.1f));
	if(falloff > 0.0f)
	{
		//float d = (noise3d(p * 0.004f) * noise3d(p * 0.016f)) * 0.006f + .01f;
		//float d = 0.010f;
		d += saturate((-p.y - 2.0f) * 0.0003f);
		//d = saturate(d);
		//fogd = (abs(noise3d(p * 0.046f)) + abs(noise3d(p * 0.021f) * 8.0f)) * 0.4f + 0.1f;
		fogd = abs(noise3d(p * 0.1261f)) * 0.2f + 0.8f;
	}
	float4 f = float4(FOG_COLOR * fogd * d, d) * dist;
	return 0.0f;
	return f;
}

/*float3 getPointFog(float3 rgb, float dist, float3 d)
{
   float fogAmount = c * exp(-Eye.y*b) * (1.0-exp( -dist*d.y*b ))/d.y;
   float3  fogColor  = vec3(0.5f,0.6f,0.7f);
   return lerp( rgb, fogColor, fogAmount );
}*/