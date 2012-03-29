
// The number of sample points taken along the ray
const static float fSamples = 3.0f;
// The number of sample points taken along the ray
const static uint nSamples = 3; 

// The scale depth (the altitude at which the average atmospheric density is found)
const static float scaleDepth = 0.21f;	
const static float eSpace = 1.0f;
const static float eSun = 30.0f;
const static float kr = 0.0075f;	// 0.0025f
const static float km = 0.0005f;
const static float pi = 3.14159265;
const static float innerRadius = 10.0f;			
const static float outerRadius = 12.0f;
const static float3 waveLength = float3(0.650f, 0.570f, 0.475f);
const static float3 waveLength4 = pow(waveLength, 4);
const static float g = -0.950f;	// The Mie phase asymmetry factor  should be between -0.75 and -0.999 
float3 getSpace(float3 dir)
{	
	//to remove artifacts on horizon
	if(dir.y <= 0.0f) return 0.0f;	
	
	float3 space = 0.0f;
	space = noise3d(dir * 100.445f);
	space = noise3d(dir * 100.445f);
	space -= abs(noise3d(dir * 13.2f));
	space -= abs(noise3d(dir * 19.2f));
	space -= abs(noise3d(dir * 49.2f));
	space -= abs(noise3d(dir * 38.2f));

	return saturate(space) * eSpace;
}


// The scale equation calculated by Vernier's Graphical Analysis
float scale(float fCos)
{
	float x = 1.0f - fCos;
	return scaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25))));
}

// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2)
{
	return 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fCos2) / pow(abs(1.0f + g2 - 2.0f*g*fCos), 1.5f);
}

// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2)
{
	return 0.75f + 0.75f*fCos2;
}

float3 applyPhase(
			const float3 c0, //Rayleigh color
			const float3 c1, // The Mie color
			const float3 camDir	//camdirection (inverted?)
			)
{
	float fCos = dot(SunDirection, camDir) / length(camDir);
	float fCos2 = fCos*fCos;
	float3 color = getRayleighPhase(fCos2) * c0 + getMiePhase(fCos, fCos2, g, g*g) * c1;
	return color;
}


const static float3 v3InvWavelength = 1.0f / waveLength4;
const static float fKrESun = eSun * kr; 
const static float fKmESun = eSun * km; 
const static float fKr4PI = kr * 4.0f * pi;
const static float fKm4PI = km * 4.0f * pi; 
const static float fScale = 1.0f / (outerRadius - innerRadius); 
const static float scaleOverScaleDepth	= fScale /scaleDepth; 
	
float3 getSkyColor(float3 rayDir)
{
	//Compute cameraHeigth
	float camHeight	= innerRadius;	
	camHeight += (Eye.y* 0.001f);										
	camHeight = max(camHeight, 0.0f);
	
	const float distToTop = outerRadius - camHeight;
	const float far = distToTop + (1.0f - rayDir.y) * distToTop * 2.0f;
	
	// Calculate the ray's starting position, then calculate its scattering offset
	const float3 start = float3((Eye.x* 0.001f), camHeight, (Eye.z * 0.001f));
	float depth = exp(scaleOverScaleDepth * (innerRadius - camHeight));

	float fStartAngle = dot(rayDir, normalize(start));
	float fStartOffset = depth*scale(fStartAngle);

	// Initialize the scattering loop variables
	float sampleLength = far / fSamples;					
	float scaledLength = sampleLength * fScale;				
	float3 sampleRay = rayDir * sampleLength; 
	float3 samplePoint = start + sampleRay * 0.5f; 

	//Loop through the sample rays
	float3 v3FrontColor = float3(0.0f, 0.0f, 0.0f);
	for(uint i=0; i<nSamples; i++)
	{
		float height = length(samplePoint);
		float depth = exp(scaleOverScaleDepth * (innerRadius - height));
			
		float fLightAngle = dot(SunDirection, samplePoint) / height;
		float fCameraAngle = dot(rayDir, samplePoint) / height;

		float fScatter = (fStartOffset + depth*(scale(fLightAngle) - scale(fCameraAngle)));

		float3 attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));	
		
		v3FrontColor += attenuate * (depth * scaledLength);
		samplePoint += sampleRay;
	}
	
	//scale the Mie and Rayleigh colors
	float3 mieC = v3FrontColor * (v3InvWavelength * fKrESun);
	float3 rayleighC = v3FrontColor * fKmESun;
	float3 t = -rayDir*far;

	return applyPhase(mieC, rayleighC ,t);
}

float3 getSky(float3 rayDir)
{
	rayDir = normalize(float3(rayDir.x, saturate(rayDir.y), rayDir.z));
	float3 skyColor = getSkyColor(rayDir);
	float3 spaceColor = getSpace(rayDir) * saturate(-SunDirection.y); 	
	float hemi = saturate(dot(rayDir, SunDirection));
	float col = pow(hemi,300.0f);
	
	return skyColor + spaceColor + col  * 5.0f;
	
	
	return skyColor + spaceColor ;
	/*old sky
	float3 c = float3(0.0f, 0.0f, 1.0f - rayDir.y * 0.6f);
	c.rg += (c.b - 0.6f) * 1.0f;
	float hemi = saturate(dot(rayDir, SunDirection));

	return lerp(c.rgb, float3(2.0f, 2.0f, 1.2f), pow(hemi, 14.0f));
	*/
}