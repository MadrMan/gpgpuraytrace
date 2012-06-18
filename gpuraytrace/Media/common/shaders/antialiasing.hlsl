//Multisample antialiasing

//Define in program to control sample count
#ifndef AA_SAMPLES
#define AA_SAMPLES 2 //only one sample is default
#endif

//Offsets taken from http://msdn.microsoft.com/en-us/library/windows/desktop/ff476218.aspx (D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS enumeration)
const static float2 AA_SAMPLE_OFFSETS[] = { 
#if AA_SAMPLES == 1
	float2(0.0f, 0.0f)
#elif AA_SAMPLES == 2
	float2(4.0f, 4.0f),
	float2(-4.0f, -4.0f)
#elif AA_SAMPLES == 4
	float2(-2.0f, -6.0f),
	float2(6.0f, -2.0f),
	float2(-6.0f, 2.0f),
	float2(2.0f, 6.0f)
#elif AA_SAMPLES == 8
	float2(1.0f, -3.0f),
	float2(-1.0f, 3.0f),
	float2(5.0f, 1.0f),
	float2(-3.0f, -5.0f),
	float2(-5.0f, 5.0f),
	float2(-7.0f, -1.0f),
	float2(3.0f, 7.0f),
	float2(7.0f, -7.0f),
#elif AA_SAMPLES == 16
	float2(1.0f, 1.0f),
	float2(-1.0f, 3.0f),
	float2(-3.0f, 2.0f),
	float2(4.0f, -1.0f),
	float2(-5.0f, -2.0f),
	float2(2.0f, 5.0f),
	float2(5.0f, 3.0f),
	float2(3.0f, -5.0f),
	float2(-2.0f, 6.0f),
	float2(0.0f, -7.0f),
	float2(-4.0f, -6.0f),
	float2(-6.0f, 4.0f),
	float2(-8.0f, 0.0f),
	float2(7.0f, -4.0f),
	float2(6.0f, 7.0f),
	float2(-7.0f, -8.0f),
#else
	#error Unsupported AA sample count (only 1, 2, 4, 8, 16 supported)
#endif
};

uint getSampleCount()
{
	return AA_SAMPLES;
}

float2 getSampleOffset(uint num)
{
	return AA_SAMPLE_OFFSETS[num] / 16.0f;
}