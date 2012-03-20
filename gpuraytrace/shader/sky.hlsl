// The number of sample points taken along the ray
const static float fSamples = 3.0f;
// The number of sample points taken along the ray
const static uint nSamples = 3;
// The scale depth (the altitude at which the average atmospheric density is found)
const static float fScaleDepth = 0.25f; 			

// The scale equation calculated by Vernier's Graphical Analysis
float scale(float fCos)
{
	float x = 1.0f - fCos;
	return fScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25))));
}

// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2)
{
	return 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fCos2) / pow(1.0f + g2 - 2.0f*g*fCos, 1.5f);
}
// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2)
{
	//return 1.0;
	return 0.75f + 0.75f*fCos2;
}

const static float g = -0.999f;		//-0.75 and -0.999 do
float3 applyPhase(
			float4 c0,				//Rayleigh color
			float4 c1,  			// The Mie color
			float3 v3Direction,		//camdirection (inverted?)
			float3 v3LightPos		//sundirection
			)
{
	float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
	float fCos2 = fCos*fCos;
	float3 color = getRayleighPhase(fCos2) * c0 + getMiePhase(fCos, fCos2, g, g*g) * c1;

	return color;
}


const static float ESun = 100.0f; 			//20.0f;
const static float kr   = 0.0025f;
const static float km   = 0.0010f;
const static float pi   = 3.14159265;
const static float innerRadius = 10.0f;			
const static float outerRadius = 12.0f;		//10.25f;
const static float3 waveLength = float3(0.650f, 0.570f, 0.475f);
const static float3 waveLength4 = pow(waveLength, 4);

const static float3 v3InvWavelength = 1.0f / waveLength4; // 1 / pow(wavelength, 4) for the red, green, and blue channels
const static float fKrESun = ESun * kr; // Kr * ESun
const static float fKmESun = ESun * km; // Km * ESun
const static float fKr4PI = kr * 4.0f * pi; // Kr * 4 * PI
const static float fKm4PI = km * 4.0f * pi; // Km * 4 * PI
const static float fScale = 1.0f / (outerRadius - innerRadius); // 1 / (fOuterRadius - fInnerRadius)
const static float fScaleOverScaleDepth	= fScale /fScaleDepth; // fScale / fScaleDepth
	
float3 getSkyColor(float3 rayDir)
{
	float fCameraHeight		= 	innerRadius;							// The camera's current height
	
	fCameraHeight += (Eye.y* 0.001f);										
	fCameraHeight = max(fCameraHeight, 0.0f);
	
	const float fDistToTop = outerRadius - fCameraHeight;
	const float fFar = fDistToTop + (1.0f - rayDir.y) * fDistToTop * 2.0f;
	
	// Calculate the ray's starting position, then calculate its scattering offset
	const float3 v3Start = float3((Eye.x* 0.001f), fCameraHeight, (Eye.z * 0.001f));
	float fDepth = exp(fScaleOverScaleDepth * (innerRadius - fCameraHeight));

	float fStartAngle = dot(rayDir, normalize(v3Start));
	float fStartOffset = fDepth*scale(fStartAngle);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;					
	float fScaledLength = fSampleLength * fScale;				
	float3 v3SampleRay = rayDir * fSampleLength;			//Direction dampleray
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;		//richting 

	//Loop through the sample rays
	float3 v3FrontColor = float3(0.0f, 0.0f, 0.0f);
	for(uint i=0; i<nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (innerRadius - fHeight));
			
		float fLightAngle = dot(SunDirection, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(rayDir, v3SamplePoint) / fHeight;

		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));

		float3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));	
		
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	
	//scale the Mie and Rayleigh colors
	float4 c0 = float4(v3FrontColor * (v3InvWavelength * fKrESun),0.0f);
	float4 c1 = float4(v3FrontColor * fKmESun,0.0f);
	float3 t0 = -rayDir*fFar;

	return applyPhase(c0, c1 ,t0 , SunDirection);
}

float3 getSky(float3 rayDir)
{
	//return float3(1,0,0);
	return getSkyColor( rayDir).rgb;
	//old sky	
	float3 c = float3(0.0f, 0.0f, 1.0f - rayDir.y * 0.6f);
	c.rg += (c.b - 0.6f) * 1.0f;
	float hemi = saturate(dot(rayDir, SunDirection));
	return lerp(c.rgb, float3(2.0f, 2.0f, 1.2f), pow(hemi, 14.0f));
}