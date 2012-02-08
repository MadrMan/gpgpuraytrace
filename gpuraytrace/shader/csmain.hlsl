cbuffer textureInfo
{
	int2 size;
}

RWStructuredBuffer<float4> texOut : register(u0);

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	texOut[DTid.y * size.x + DTid.x] = float4(0.0f, DTid.x * 0.01f, DTid.y * 0.01, 0.0f);
}