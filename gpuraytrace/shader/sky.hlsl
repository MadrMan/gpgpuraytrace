

// The number of sample points taken along the ray
const static float fSamples = 3;
// The number of sample points taken along the ray
const static uint nSamples = 3;
// The scale depth (the altitude at which the average atmospheric density is found)
const static float fScaleDepth = 0.25; 			


// The scale equation calculated by Vernier's Graphical Analysis
float scale(float fCos)
{
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}


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
float4 getSkyColor(
			float4 c0,			//Rayleigh color
			float4 c1,  		// The Mie color
			float3 v3Direction,		//camdirection (inverted?)
			float3 v3LightPos		//sundirection
			)
{
	float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);
	float fCos2 = fCos*fCos;
	float4 color = getRayleighPhase(fCos2) * c0 + getMiePhase(fCos, fCos2, g, g*g) * c1;
	color.a = color.b;
	return color;
}


const static float ESun = 20.0f;
const static float kr   = 0.0025f;
const static float km   = 0.0010f;
const static float pi   = 3.14159265;
const static float innerRadius = 10.0f;			//is dit in km??
const static float outerRadius = 10.25f;
const static float waveLenghtR = 0.650f;
const static float waveLengthG = 0.570f;
const static float waveLengthB = 0.475f;

float4 blaat(float3 dir, float3 camPos)
{
	float3 v3CameraPos = camPos;		// The camera's current position
	float3 v3LightPos = SunDirection;	// The direction vector to the light source
	
	float3 waveLenght4 = float3(pow(waveLenghtR,4), pow(waveLengthG,4), pow(waveLengthB,4));

	float3 v3InvWavelength 	= 	1.0f / waveLenght4; 		// 1 / pow(wavelength, 4) for the red, green, and blue channels
		
	float fKrESun			=	ESun * kr;			// Kr * ESun
	float fKmESun			=	ESun * km;			// Km * ESun
	float fKr4PI			= 	kr * 4.0f * pi;		// Kr * 4 * PI
	float fKm4PI			=	km * 4.0f * pi;		// Km * 4 * PI
	float fScale 			= 	1.0f / (outerRadius - innerRadius);	// 1 / (fOuterRadius - fInnerRadius)
	float fScaleOverScaleDepth	=	fScale /fScaleDepth;						// fScale / fScaleDepth
	float fCameraHeight		= 	10.02f;				// The camera's current height
	
	float fDistToTop = outerRadius - fCameraHeight;
	float fFar = (outerRadius - fCameraHeight) + (1.0f - dir.y) * fDistToTop * 2.0f;
	//float fFar = 10.11f;
	float3 v3Ray = dir; // normalized cam direction 

	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = float3(0, fCameraHeight, 0); //camPos
	float fHeight = length(v3Start);
	float fDepth = exp(fScaleOverScaleDepth * (innerRadius - fCameraHeight));

	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth*scale(fStartAngle);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;						//lengte 1 sample
	float fScaledLength = fSampleLength * fScale;				
	float3 v3SampleRay = v3Ray * fSampleLength;					//richting sampleray eerste deel
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;			//richting 

	// Now loop through the sample rays

	float3 v3FrontColor = float3(0.0, 0.0, 0.0);
	for(uint i=0; i<nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (innerRadius - fHeight));
			
		float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;

		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));

		float3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));	
		
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	//vertout OUT;
	//OUT.pos = mul(gl_ModelViewProjectionMatrix, gl_Vertex); 
	float4 c0 = float4(v3FrontColor * (v3InvWavelength * fKrESun),0.0f);
	float4 c1 = float4(v3FrontColor * fKmESun,0.0f);
	float3 t0 = -dir*fFar;
	//return (c1 + c0) *100;
	//return float4(v3FrontColor,0.0f);
	//return c0;
	return getSkyColor(c0, c1 ,t0 , SunDirection);
}


float3 getSky(float3 dir, float3 camPos)
{

	return blaat( dir, camPos ).rgb;
	
	float4 result = getSkyColor( float4(0,0,1,0), float4(1,1,1,0), dir, -1*SunDirection);
	return result.xyz;
	
	float3 c = float3(0.0f, 0.0f, 1.0f - dir.y * 0.6f);
	c.rg += (c.b - 0.6f) * 1.0f;
	float hemi = saturate(dot(dir, SunDirection));
	return lerp(c.rgb, float3(2.0f, 2.0f, 1.2f), pow(hemi, 14.0f));
}