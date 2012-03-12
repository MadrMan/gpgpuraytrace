// The number of sample points taken along the ray
//const int nSamples = 2;
//const float fSamples = (float)nSamples;

// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2)
{
	return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
}

// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2)
{
	//return 1.0;
	return 0.75 + 0.75*fCos2;
}





const static float g = -0.75;
float3 getSky(float3 dir)
{
	//float4 result = getSkyColor( float4(0,0,0,0), float4(1,0,0,0), dir, SunDirection, g , g * g);
	//return result.xyz;
	
	float3 c = float3(0.0f, 0.0f, 1.0f - dir.y * 0.6f);
	c.rg += (c.b - 0.6f) * 1.0f;
	float hemi = saturate(dot(dir, SunDirection));
	return lerp(c.rgb, float3(2.0f, 2.0f, 1.2f), pow(hemi, 14.0f));
}