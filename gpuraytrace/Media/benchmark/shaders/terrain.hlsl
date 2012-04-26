float getDensity(float3 p)
{
	float d = 0.0f;

	/*p += float3(
		noise3d(p * 0.00682),
		noise3d(p * 0.00474),
		noise3d(p * 0.00641)) * 16.0f;*/
	p *= 0.1f;
	d += -p.y;		
	d += noise3d(p * 0.124f) * 8.362f;
	d += noise3d(p * 0.237f) * 4.162f;
	d += noise3d(p * 0.52f) * 2.062f;
	d += noise3d(p * 1.013) * 1.04;
	d += noise3d(p * 2.01f) * 0.5f;
	d += noise3d(p * 4.13f) * 0.25f;
	d += noise3d(p * 8.04f) * 0.125f;
	d += noise3d(p * 16.018f) * 0.063f;
	/*d += noise3d(p * 2.213f) * 1.16f;
	d += noise3d(p * 1.278f) * 3.12f;
	d += noise3d(p * 0.645f) * 6.21f;
	d += noise3d(p * 0.246f) * 14.0f;*/
	//d += pow((abs(noise3d(p * 0.047f)) + 0.6f) * 6.0f, 2.0f) * noise3d(p * 0.02475f);
	d += pow((1.0f - saturate(abs(-1.0 - p.y) * 0.3f)) * 2.0f, 2.3f);
	//d -= saturate(-p.y) * 4.0f;
	return d;
}

const static float3 FOG_COLOR = float3(0.9f, 0.9f, 0.9f);
float4 getFog(float3 p, float dist)
{

	float fogd = 0.0f;
	float d = 0.0f;
	
	dist = 1.0f - dist * 0.0012f;						
	float falloff = 1.0f - (dist * dist * 0.1f);	

	fallof = 
	if(falloff > 0.0f)
	{
		//float d = (noise3d(p * 0.004f) * noise3d(p * 0.016f)) * 0.006f + .01f;
		//float d = 0.010f;
		d += saturate((-p.y - 2.0f) * 0.0003f);
		//d = saturate(d);
		//fogd = (abs(noise3d(p * 0.046f)) + abs(noise3d(p * 0.021f) * 8.0f)) * 0.4f + 0.1f;
		fogd = abs(noise3d(p * 0.1261f)) * 0.2f + 0.8f;
	}
	
	return  float4(FOG_COLOR * fogd * d, d) * dist;
}

/*float3 getPointFog(float3 rgb, float dist, float3 d)
{
   float fogAmount = c * exp(-Eye.y*b) * (1.0-exp( -dist*d.y*b ))/d.y;
   float3  fogColor  = vec3(0.5f,0.6f,0.7f);
   return lerp( rgb, fogColor, fogAmount );
}*/