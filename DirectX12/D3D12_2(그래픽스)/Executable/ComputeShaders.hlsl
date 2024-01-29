cbuffer cbSettings : register(b10)
{
	// Support up to 11 blur weights.
    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
};
static const int gBlurRadius = 5;

Texture2D<float4> gInput : register(t14);
RWTexture2D<float4> gOutput : register(u1);

#define N 256
#define CacheSize (N + 2*gBlurRadius)

groupshared float4 gCache[CacheSize];// 스레드 그룹 공유자원

[numthreads(256, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (groupThreadID.x < gBlurRadius)
    {
		// clamp해주고
        int x = max(dispatchThreadID.x - gBlurRadius, 0);
        gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
    }
    if (groupThreadID.x >= N - gBlurRadius)
    {
        int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
        gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
    }
    
    gCache[groupThreadID.x + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];
    
    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = groupThreadID.x + gBlurRadius + i;
		
        blurColor += weights[i + gBlurRadius] * gCache[k];
    }
	
    gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, 256, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID,
				int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

    if (groupThreadID.y < gBlurRadius)
    {
		// Clamp out of bound samples that occur at image borders.
        int y = max(dispatchThreadID.y - gBlurRadius, 0);
        gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= N - gBlurRadius)
    {
		// Clamp out of bound samples that occur at image borders.
        int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y - 1);
        gCache[groupThreadID.y + 2 * gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
    }
	
    gCache[groupThreadID.y + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];


    GroupMemoryBarrierWithGroupSync();
	
    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = groupThreadID.y + gBlurRadius + i;
		
        blurColor += weights[i + gBlurRadius] * gCache[k];
    }
	
    gOutput[dispatchThreadID.xy] = blurColor;
}