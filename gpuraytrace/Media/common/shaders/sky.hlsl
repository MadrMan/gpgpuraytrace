// The number of sample points taken along the ray
const static float fSamples = 3.0f;
// The number of sample points taken along the ray
const static uint nSamples = 3; 
// The scale depth (the altitude at which the average atmospheric density is found)
const static float scaleDepth = 0.19f;
const static float eSpace = 1.0f;
const static float eSun = 12.0f;
const static float kr = 0.003f;
const static float km = 0.0025f;
const static float pi = 3.14159265;
const static float innerRadius = 200.0f;			
const static float outerRadius = innerRadius * 1.025f; 
const static float3 waveLength = float3(0.650f, 0.570f, 0.475f);
const static float3 waveLength4 = pow(waveLength, 4);
const static float g = -0.99f;	// The Mie phase asymmetry factor  should be between -0.75 and -0.999 

float3 modRayDir(float3 rayDir)
{
	//Continue drawing sky so it can be used to colorize landscape
	float y =  saturate(rayDir.y);
	return  normalize(float3(rayDir.x, y, rayDir.z));
}


float3 getSpaceColor(float3 dir)
{	
	dir = modRayDir(dir);
	//to remove line artifacts on horizon
	if(dir.y <= 0.0f) return 0.0f;	
	
	float3 space = 0.0f;
	space = noise3d(dir * 100.445f);
	space -= abs(noise3d(dir * 13.2f));
	space -= abs(noise3d(dir * 19.2f));
	space -= abs(noise3d(dir * 49.2f));
	space -= abs(noise3d(dir * 38.2f));
	space *= 2.0f;
	space = saturate(space) * eSpace * saturate(-SunDirection.y);
	return space;
}

// Scale equation by Vernier's Graphical Analysis
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

struct SkyColor
{
	float3 mie;
	float3 rayleigh;
};

//Apply phase on rayleigh and mie scattering
SkyColor applyPhase(const float3 rayleigh, const float3 mie, const float3 camDir)
{
	float fCos = dot(SunDirection, camDir) / length(camDir);
	float fCos2 = fCos*fCos;	
	SkyColor color;
	color.mie = getMiePhase(fCos, fCos2, g, g*g) * mie;
	color.rayleigh = getRayleighPhase(fCos2) * rayleigh;
	return color;
}

const static float3 v3InvWavelength = 1.0f / waveLength4;
const static float fKrESun = eSun * kr; 
const static float fKmESun = eSun * km; 
const static float fKr4PI = kr * 4.0f * pi;
const static float fKm4PI = km * 4.0f * pi; 
const static float fScale = 1.0f / (outerRadius - innerRadius); 
const static float scaleOverScaleDepth	= fScale /scaleDepth; 

//Retrieve sky color.	
SkyColor getRayleighMieColor(float3 orgRayDir)
{
	float3 rayDir = modRayDir(orgRayDir);
	
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
	float3 mie = v3FrontColor * (v3InvWavelength * fKrESun);
	float3 rayleigh = v3FrontColor * fKmESun;
	float3 t = -rayDir*far;
	
	SkyColor sc = applyPhase(mie, rayleigh ,t);
	sc.rayleigh *= saturate((orgRayDir.y * 0.5f + 0.5f) * 4.0f); 
	sc.rayleigh.b += 0.6f * saturate(SunDirection.y);	// brighten the day hack
	sc.rayleigh.g += 0.4f * saturate(SunDirection.y);	
	sc.rayleigh.r += 0.3f * saturate(SunDirection.y);	
	return sc;
}


/*
//computes the complete sky color
float3 getSky(float3 rayDir)
{
	SkyColor scat = getRayleighMieColor(rayDir);
	float3 spaceColor = getSpaceColor(rayDir);
	return scat.mie + scat.rayleigh + spaceColor;	
	//old sky
	//float3 c = float3(0.0f, 0.0f, 1.0f - rayDir.y * 0.6f);
	//c.rg += (c.b - 0.6f) * 1.0f;
	//float hemi = saturate(dot(rayDir, SunDirection));
	//return lerp(c.rgb, float3(2.0f, 2.0f, 1.2f), pow(hemi, 14.0f));
}
*/