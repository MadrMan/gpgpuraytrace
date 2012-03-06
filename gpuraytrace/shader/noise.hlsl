SamplerState state;

#if 0
static const float3 grad3[] = {{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}};

static const uint perm[] = {151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

// 2D simplex noise
float noise2d(float2 pin) 
{
	pin *= 0.3f; //Scale to match texture

	float n0, n1, n2; // Noise contributions from the three corners
	// Skew the input space to determine which simplex cell we're in
	static const float F2 = 0.5f*(sqrt(3.0f)-1.0f);
	float s = (pin.x+pin.y)*F2; // Hairy factor for 2D
	int i = floor(pin.x+s);
	int j = floor(pin.y+s);
	static const float G2 = (3.0f-sqrt(3.0f))/6.0f;
	float t = (i+j)*G2;
	float X0 = i-t; // Unskew the cell origin back to (x,y) space
	float Y0 = j-t;
	float x0 = pin.x-X0; // The x,y distances from the cell origin
	float y0 = pin.y-Y0;
	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	uint i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)
	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6
	float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0f + 2.0f * G2; // Offsets for last corner in (x,y) unskewed coords
	float y2 = y0 - 1.0f + 2.0f * G2;
	// Work out the hashed gradient indices of the three simplex corners
	uint ii = i & 255;
	uint jj = j & 255;
	uint gi0 = perm[ii+perm[jj]] % 12;
	uint gi1 = perm[ii+i1+perm[jj+j1]] % 12;
	uint gi2 = perm[ii+1+perm[jj+1]] % 12;
	// Calculate the contribution from the three corners
	float t0 = 0.5f - x0*x0-y0*y0;
	if(t0<0.0f) n0 = 0.0f;
	else {
	t0 *= t0;
	n0 = t0 * t0 * dot(grad3[gi0].xy, float2(x0, y0)); // (x,y) of grad3 used for 2D gradient
	}
	float t1 = 0.5f - x1*x1-y1*y1;
	if(t1<0.0f) n1 = 0.0f;
	else {
	t1 *= t1;
	n1 = t1 * t1 * dot(grad3[gi1].xy, float2(x1, y1));
	}
	float t2 = 0.5f - x2*x2-y2*y2;
	if(t2<0.0f) n2 = 0.0f;
	else {
	t2 *= t2;
	n2 = t2 * t2 * dot(grad3[gi2].xy, float2(x2, y2));
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 70.0f * (n0 + n1 + n2);
}

#endif
#if 0
Texture2D<float> texNoise;
const static float TEXSIZE = 128.0f;

float2 fade(float2 t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

const static int TEXSIZEI = 128;
float noise2d(float2 pos)
{
	pos = abs(pos);
	int2 loc = floor(pos);
	float4 samples = float4(
		texNoise.Load(uint3(loc, 0.0f) % TEXSIZEI), 
		texNoise.Load(uint3((loc + uint2(1, 0)) % TEXSIZEI, 0.0f)), 
		texNoise.Load(uint3((loc + uint2(1, 1)) % TEXSIZEI, 0.0f)), 
		texNoise.Load(uint3((loc + uint2(0, 1)) % TEXSIZEI, 0.0f)));
	float2 fract = fade(pos - loc.xy);
	//float2 fract = fmod(pos - loc.xy, 1.0f);
	float2 interpx = float2(
		lerp(samples.x, samples.y, fract.x),
		lerp(samples.w, samples.z, fract.x));
	//return lerp(samples.x, samples.y, fract.x) * 0.5f + samples.x * 0.5f;
	return lerp(interpx.x, interpx.y, fract.y) * 2.0f - 1.0f;
}
#endif
#if 1

static const int TEXTURE_SIZE = 128;
//static const float ONE_PIXEL = 1.0f / (float)TEXTURE_SIZE;
static const uint ONE_PIXEL = 1;

cbuffer CBNoise
{
	float4 permGradients[TEXTURE_SIZE];
}

Texture2D<uint4> texPerm2D : register(t0);
//Texture1D<float4> texPermGrad : register(t1);

//3D
float3 fade(float3 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

//3D
float gradperm(uint x, float3 p)
{
	//return dot(texPermGrad.SampleLevel(state, x, 0).xyz, p);
	//return dot(texPermGrad.Load(uint2(x % TEXTURE_SIZE, 0)).xyz, p);
	return dot(permGradients[x % TEXTURE_SIZE].xyz, p);
}

// 3D noise
float noise3d(float3 p)
{
	int3 P = floor(p);
	p -= P;
	float3 f = fade(p);

	uint4 Pu = uint4(
		P.x < 0 ? (TEXTURE_SIZE - (uint)abs(P.x)) % TEXTURE_SIZE : (uint)P.x % TEXTURE_SIZE,
		P.y < 0 ? (TEXTURE_SIZE - (uint)abs(P.y)) % TEXTURE_SIZE : (uint)P.y % TEXTURE_SIZE,
		P.z < 0 ? (TEXTURE_SIZE - (uint)abs(P.z)) % TEXTURE_SIZE : (uint)P.z % TEXTURE_SIZE,
		0
	);
	
	Pu = texPerm2D.Load(uint3(Pu.xy, 0)) + Pu.z;
	//AA /= TEXTURE_SIZE;
	
	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
  	return lerp( lerp( lerp( gradperm(Pu.x, p ),  
                             gradperm(Pu.z, p + float3(-1, 0, 0) ), f.x),
                       lerp( gradperm(Pu.y, p + float3(0, -1, 0) ),
                             gradperm(Pu.w, p + float3(-1, -1, 0) ), f.x), f.y),
                             
                 lerp( lerp( gradperm(Pu.x + ONE_PIXEL, p + float3(0, 0, -1) ),
                             gradperm(Pu.z + ONE_PIXEL, p + float3(-1, 0, -1) ), f.x),
                       lerp( gradperm(Pu.y + ONE_PIXEL, p + float3(0, -1, -1) ),
                             gradperm(Pu.w + ONE_PIXEL, p + float3(-1, -1, -1) ), f.x), f.y), f.z);
}
#endif

/*RWTexture2D<float4> texOut : register(u0);
[numthreads(16, 16, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	float4 tnoise = 0.0f;
	const static int iterations = 2500;
	for(int x = 0; x < iterations; x++)
	{
		float2 texel = DTid.xy / 40.0f;
		float4 noise = noise3d(float3(texel.x, 0.0f, texel.y));
		tnoise += noise * (1.0f / iterations);
	}
	texOut[DTid.xy] = tnoise * 0.5f + 0.5f; //fmod(tnoise * 0.5f + 0.5f, 0.1f) * 10.0f;

	//float4(h.xxx * 0.5f + 0.5f, 0.0f);
}*/