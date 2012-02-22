//cbuffer textureInfo
//{
	static const int2 size = int2(800, 600);
//}

//!
cbuffer GlobalVariables
{
	float test1;
	float2 test2;
	float3 test3;
}

cbuffer PerFrame
{
	float4x4 View;
	float4x4 Projection;
}

RWTexture2D<float4> texOut : register(u0);

[numthreads(10, 10, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
	test1;
	texOut[DTid.xy] = float4(0.00f, DTid.x * test1, DTid.y * 0.01, 0.0f);
	
}