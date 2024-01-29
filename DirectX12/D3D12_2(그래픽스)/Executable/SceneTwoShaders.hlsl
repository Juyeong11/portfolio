
cbuffer cbPlayerInfo : register(b0)
{
    matrix gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView : packoffset(c0);
    matrix gmtxProjection : packoffset(c4);
    float3 gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
    matrix gmtxGameObject : packoffset(c0);
    uint gnMaterial : packoffset(c4);
};
#include "Light.hlsl"
cbuffer cbFrameworkInfo : register(b3)
{
    float gfCurrentTime;
    float gfElapsedTime;
    float2 gf2CursorPos;
};

struct CB_TOOBJECTSPACE
{
    matrix mtxToTexture;
    float4 f4Position;
};

cbuffer cbProjectorSpace : register(b5)
{
    CB_TOOBJECTSPACE gcbToProjectorSpaces[MAX_LIGHTS];
};

cbuffer cbToLightSpace : register(b6)
{
    CB_TOOBJECTSPACE gcbToLightSpaces[MAX_LIGHTS];
};

SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

Texture2D<float4> gWallTexture : register(t1);
Texture2D<float4> gDepthBufferTexture[MAX_DEPTH_TEXTURES] : register(t2);
struct VS_NORMAL_INPUT
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VS_NORMAL_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    
    float4 uvs[MAX_LIGHTS] : TEXCOORD0;
};

VS_NORMAL_OUTPUT VSNormal(VS_NORMAL_INPUT input)
{
    VS_NORMAL_OUTPUT vout = (VS_NORMAL_OUTPUT) 0.0f;

	// Fetch the material data.
   // MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Transform to world space.
    float4 posW = mul(float4(input.PosL, 1.0f), gmtxGameObject);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(input.NormalL, (float3x3) gmtxGameObject);
	
    vout.TangentW = float3(input.TexC,1);

    // Transform to homogeneous clip space.
    vout.PosH = mul(mul(posW, gmtxView), gmtxProjection);
	
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (gcbToLightSpaces[i].f4Position.w != 0.0f)
            vout.uvs[i] = mul(posW, gcbToLightSpaces[i].mtxToTexture);// 그림자 맵을 장면에 투영하기 위해 투영 텍스쳐 좌표를 만든다.
    }
	// Output vertex attributes for interpolation across triangle.
    //float4 texC = mul(float4(input.TexC, 0.0f, 1.0f), gTexTransform);
    //vout.TexC = mul(texC, matData.MatTransform).xy;
    return vout;
}

float4 PSNormal(VS_NORMAL_OUTPUT input) : SV_TARGET
{
    float4 cIllumination = Lighting(input.PosW, normalize(input.NormalW), true, input.uvs);
    float4 cColor;
    if (input.TangentW.y > 1.f)
    {
        cColor = float4(1.f, 1.f, 1.f, 1.f);
    }
    else
    {  
        cColor = gWallTexture.Sample(gWrapSamplerState, input.TangentW.xy);
    }
    
    float4 ret = cColor * cIllumination;
    ret.a = cIllumination.a;
    return (ret);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


//VS_NORMAL_OUTPUT VS(VS_NORMAL_INPUT input)
//{
//    VS_NORMAL_OUTPUT output;

//    output.PosH = mul(mul(mul(float4(input.PosL, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
//    output.NormalW = float4(input.NormalL, 1);

//    return (output);
//}

//float4 PS(VS_NORMAL_OUTPUT input) : SV_TARGET
//{
//    return float4(input.NormalW, 1 );
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_DIFFUSED_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VS_DIFFUSED_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VS(VS_DIFFUSED_INPUT input)
{
    VS_DIFFUSED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.color = float4(input.normal, 1);

    return (output);
}

float4 PS(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
    return (input.color);
}

struct PS_DEPTH_OUTPUT
{
    float fzPosition : SV_Target;
    float fDepth : SV_Depth;
};

PS_DEPTH_OUTPUT PSDepthWriteShader(VS_NORMAL_OUTPUT input)
{
    PS_DEPTH_OUTPUT output;

    output.fzPosition = input.PosH.z;
    output.fDepth = input.PosH.z;

    return (output);
}

///////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToViewport(uint nVertexID : SV_VertexID)
{
    VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT) 0;

    if (nVertexID == 0)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    if (nVertexID == 1)
    {
        output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 0.0f);
    }
    if (nVertexID == 2)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    if (nVertexID == 3)
    {
        output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 0.0f);
    }
    if (nVertexID == 4)
    {
        output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(1.0f, 1.0f);
    }
    if (nVertexID == 5)
    {
        output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        output.uv = float2(0.0f, 1.0f);
    }

    return (output);
}

SamplerState gssBorder : register(s3);

float4 PSTextureToViewport(VS_TEXTURED_OUTPUT input) : SV_Target
{
    float4 cColor = gDepthBufferTexture[1].SampleLevel(gssBorder, input.uv, 0).r * 1.0f;

//	cColor = GetColorFromDepth(cColor.r);

    return (cColor);
}