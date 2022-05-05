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
    float gfAnimationTime : packoffset(c4);
};

cbuffer cbFrameworkInfo : register(b3)
{
    float gfCurrentTime;
    float gfElapsedTime;
    float2 gf2CursorPos;
};

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

VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
    VS_DIFFUSED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
    output.color = float4(input.normal, 1);

    return (output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
    return (input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D<float4> gtxtTexture : register(t0);
Texture2D<float4> gtxtTexture1 : register(t1);

SamplerState gWrapSamplerState : register(s0);

struct VS_TEXTURED_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
    output.uv = input.uv;

    return (output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture.Sample(gWrapSamplerState, input.uv);


    return (cColor);
}


VS_TEXTURED_OUTPUT VSTextured1(VS_TEXTURED_INPUT input)
{
    VS_TEXTURED_OUTPUT output;

    output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);

    uint nResidual = (uint) (gfAnimationTime);


    output.uv.x = input.uv.x + (nResidual % 5) * 0.20f;
    output.uv.y = input.uv.y + (nResidual / 5) * 0.25f;

    return (output);
}
float4 PSTextured1(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtTexture1.Sample(gWrapSamplerState, input.uv);


    return (cColor);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D<float4> gtxtTerrainBaseTexture : register(t2);
Texture2D<float4> gtxtTerrainDetailTextures[2] : register(t3); //t3, t4, t5
//Texture2D<float> gtxtTerrainAlphaTexture : register(t5);
Texture2D<float4> gtxtTerrainAlphaTexture : register(t5);

struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
    float3 position : POSITION;
    float3 positionW : POSITION1;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};
struct HS_TERRAIN_OUTPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct HS_TERRAIN_CONSTANT_OUTPUT
{
    float fTessEdges[4] : SV_TessFactor;
    float fTessInsides[2] : SV_InsideTessFactor;
};

struct DS_TERRAIN_OUPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float4 tessellation : TEXCOORD2;
};
VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    output.position = input.position;
    output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;

    return (output);
}
float CalculateTessFactor(float3 p)
{
    float fDistToCamera = distance(p, gvCameraPosition);
    float s = saturate((fDistToCamera) / (200.0f));

    return (lerp(64.0f, 1.0f, s));
}
HS_TERRAIN_CONSTANT_OUTPUT ConstantHS(InputPatch<VS_TERRAIN_OUTPUT, 25> input)
{
    HS_TERRAIN_CONSTANT_OUTPUT output;
    
    //변들의 중점
    float3 e0 = 0.5f * (input[0].positionW + input[4].positionW);
    float3 e1 = 0.5f * (input[0].positionW + input[20].positionW);
    float3 e2 = 0.5f * (input[4].positionW + input[24].positionW);
    float3 e3 = 0.5f * (input[20].positionW + input[24].positionW);
    
    //카메라와 거리에 따라 각 변을 쪼개는 개수를 정한다.
    output.fTessEdges[0] = CalculateTessFactor(e0);
    output.fTessEdges[1] = CalculateTessFactor(e1);
    output.fTessEdges[2] = CalculateTessFactor(e2);
    output.fTessEdges[3] = CalculateTessFactor(e3);
    
    float3 f3Sum = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 25; ++i)
        f3Sum += input[i].positionW;
    float3 f3Center = f3Sum / 25.0f;
    //수직, 수평으로 패치 내부의 테셀레이션 정도를 정한다.
    output.fTessInsides[0] = output.fTessInsides[1] = CalculateTessFactor(f3Center);
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(25)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HS_TERRAIN_OUTPUT HSTerrain(InputPatch<VS_TERRAIN_OUTPUT, 25> input, uint i : SV_OutputControlPointID)
{
    HS_TERRAIN_OUTPUT output;
    //SV_OutputControlPointID
    //헐 쉐이더에서 현재 사용하는 출력 제어점의 인덱스를 정의
    //제어점의 개수를 늘리는 용도로 사용되지만 지금은 그냥 그대로 출력함
    //  -> ..입력패치의 제어점 개수와 출력 제어점 개수가 반드시 일치해야하는 것은 아니다.
    output.position = input[i].position;
    output.color = input[i].color;
    output.uv0 = input[i].uv0;
    output.uv1 = input[i].uv1;
    return output;
}
void BernsteinCoeffcient5x5(float t, out float fBernstein[5])// 제어점 5개에 대한 베지에 곡선 공식 베른슈타인 기저 함수
{
    float tInv = 1.0f - t;
    fBernstein[0] = tInv * tInv * tInv * tInv;
    fBernstein[1] = 4.0f * t * tInv * tInv * tInv;
    fBernstein[2] = 6.0f * t * t * tInv * tInv;
    fBernstein[3] = 4.0f * t * t * t * tInv;
    fBernstein[4] = t * t * t * t;
}
float3 CubicBezierSum5x5(OutputPatch<HS_TERRAIN_OUTPUT, 25> patch, float uB[5], float vB[5])
{
    float3 f3Sum = float3(0.0f, 0.0f, 0.0f);
    f3Sum =  vB[0] * (uB[0] * patch[0].position +  uB[1] * patch[1].position +  uB[2] * patch[2].position +  uB[3] * patch[3].position + uB[4] * patch[4].position);
    f3Sum += vB[1] * (uB[0] * patch[5].position +  uB[1] * patch[6].position +  uB[2] * patch[7].position +  uB[3] * patch[8].position + uB[4] * patch[9].position);
    f3Sum += vB[2] * (uB[0] * patch[10].position + uB[1] * patch[11].position + uB[2] * patch[12].position + uB[3] * patch[13].position + uB[4] * patch[14].position);
    f3Sum += vB[3] * (uB[0] * patch[15].position + uB[1] * patch[16].position + uB[2] * patch[17].position + uB[3] * patch[18].position + uB[4] * patch[19].position);
    f3Sum += vB[4] * (uB[0] * patch[20].position + uB[1] * patch[21].position + uB[2] * patch[22].position + uB[3] * patch[23].position + uB[4] * patch[24].position);
    return (f3Sum);
}

[domain("quad")]
DS_TERRAIN_OUPUT DSTerrain(HS_TERRAIN_CONSTANT_OUTPUT input, float2 uv : SV_DomainLocation, OutputPatch<HS_TERRAIN_OUTPUT, 25> patch)
{
    DS_TERRAIN_OUPUT output = (DS_TERRAIN_OUPUT) 0;

    //테셀레이션된 정점의 위치가 uv정보로 나타나므로 실제 정점 위치를 찾아야한다.
    float uB[5], vB[5];
    BernsteinCoeffcient5x5(uv.x, uB);
    BernsteinCoeffcient5x5(uv.y, vB);

    output.color = lerp(lerp(patch[0].color, patch[4].color, uv.x), lerp(patch[20].color, patch[24].color, uv.x), uv.y);
    output.uv0 = lerp(lerp(patch[0].uv0, patch[4].uv0, uv.x), lerp(patch[20].uv0, patch[24].uv0, uv.x), uv.y);
    output.uv1 = lerp(lerp(patch[0].uv1, patch[4].uv1, uv.x), lerp(patch[20].uv1, patch[24].uv1, uv.x), uv.y);

    //실제 위치
    float3 p = CubicBezierSum5x5(patch, uB, vB);

    matrix mtxWorldViewProjection = mul(mul(gmtxGameObject, gmtxView), gmtxProjection);
    output.position = mul(float4(p, 1.0f), mtxWorldViewProjection);

    //와이어 프레임일 경우테셀레이션 정도에 따라 색을 변화시키기 위해 전달한다.
    output.tessellation = float4(input.fTessEdges[0], input.fTessEdges[1], input.fTessEdges[2], input.fTessEdges[3]);

    return (output);
}

float4 PSTerrain(DS_TERRAIN_OUPUT input) : SV_TARGET
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    
    float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv0);
//	float fAlpha = gtxtTerrainAlphaTexture.Sample(gSamplerState, input.uv0);
    float fAlpha = gtxtTerrainAlphaTexture.Sample(gWrapSamplerState, input.uv0).w;

    float4 cDetailTexColors[3];
    cDetailTexColors[0] = gtxtTerrainDetailTextures[0].Sample(gWrapSamplerState, input.uv1);
    cDetailTexColors[1] = gtxtTerrainDetailTextures[1].Sample(gWrapSamplerState, input.uv1);
    cDetailTexColors[2] = gtxtTerrainDetailTextures[2].Sample(gWrapSamplerState, input.uv1);

	//텍스쳐 블랜딩
    cColor = cBaseTexColor * cDetailTexColors[0];
    cColor += lerp(cDetailTexColors[0] * 0.25f, cDetailTexColors[1], 1.0f - fAlpha);
/* 
	cColor = lerp(cDetailTexColors[0], cDetailTexColors[2], 1.0f - fAlpha) ;
	cColor = lerp(cBaseTexColor, cColor, 0.3f) + cDetailTexColors[1] * (1.0f - fAlpha);
*/
/*
	if (fAlpha < 0.35f) cColor = cDetailTexColors[2];
	else if (fAlpha > 0.8975f) cColor = cDetailTexColors[0];
	else cColor = cDetailTexColors[1];
*/
    return (cColor);
}

float4 PSWireFrameTerrain(DS_TERRAIN_OUPUT input) : SV_TARGET
{
    float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (input.tessellation.w <= 5.0f)
        cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
    else if (input.tessellation.w <= 10.0f)
        cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
    else if (input.tessellation.w <= 20.0f)
        cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
    else if (input.tessellation.w <= 30.0f)
        cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
    else if (input.tessellation.w <= 40.0f)
        cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
    else if (input.tessellation.w <= 50.0f)
        cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    else if (input.tessellation.w <= 55.0f)
        cColor = float4(0.2f, 0.2f, 0.72f, 1.0f);
    else if (input.tessellation.w <= 60.0f)
        cColor = float4(0.5f, 0.75f, 0.75f, 1.0f);
    else
        cColor = float4(0.87f, 0.17f, 1.0f, 1.0f);
    
    return (cColor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
TextureCube gtxtSkyBox : register(t8);
SamplerState gClampSamplerState : register(s1);

struct VS_SKYBOX_INPUT
{
    float3 position : POSITION;
};

struct VS_SKYBOX_OUTPUT
{
    float3 position : POSITION;
    float4 positionH : SV_POSITION;
};
VS_SKYBOX_OUTPUT VSSkyBox(VS_SKYBOX_INPUT input)
{
    VS_SKYBOX_OUTPUT output;

    output.positionH = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection).xyww;
    output.position = input.position;

    return (output);
}

float4 PSSkyBox(VS_SKYBOX_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtSkyBox.Sample(gWrapSamplerState, input.position);

    return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define _WITH_BILLBOARD_ANIMATION

struct VS_BILLBOARD_INPUT
{
    float3 position : POSITION;
    float3 info : INFO;
};

struct VS_BILLBOARD_OUTPUT
{
    float3 position : POSITION;
    float2 size : SIZE;
    int textureID : TEXTUREID;
};

struct GS_BILLBOARD_OUTPUT
{
    float4 posH : SV_POSITION;
    float2 uv : TEXCOORD;
    int textureID : TEXTUREID;
};
#define _WITH_BILLBOARD_ANIMATION

VS_BILLBOARD_OUTPUT VSBillboard(VS_BILLBOARD_INPUT input)
{
    VS_BILLBOARD_OUTPUT output;

    //input.position.x *= (input.billboardInfo.x * 0.5f);
    //input.position.y *= (input.billboardInfo.y * 0.5f);

    //float3 f3Look = normalize(gvCameraPosition - input.instancePosition);
    //float3 f3Up = float3(0.0f, 1.0f, 0.0f);
    //float3 f3Right = normalize(cross(f3Up, f3Look));

    //matrix mtxWorld;
    //mtxWorld[0] = float4(f3Right, 0.0f);
    //mtxWorld[1] = float4(f3Up, 0.0f);
    //mtxWorld[2] = float4(f3Look, 0.0f);
    //mtxWorld[3] = float4(input.instancePosition, 1.0f);

    output.position = input.position;

    output.size = input.info.xy;

    output.textureID = (int) (input.info.z);

    return (output);
}
[maxvertexcount(4)]
void GSBillboard(point VS_BILLBOARD_OUTPUT input[1], inout TriangleStream<GS_BILLBOARD_OUTPUT> outStream)
{
    float3 vUp = float3(0.0f, 1.0f, 0.0f);
    float3 vLook = gvCameraPosition.xyz - input[0].position;
    vLook = normalize(vLook);
    float3 vRight = cross(vUp, vLook);
    float fHalfW = input[0].size.x * 0.5f;
    float fHalfH = input[0].size.y * 0.5f;
    
    float4 pVertices[4] =
    {
    
        float4(input[0].position + fHalfW * vRight - fHalfH * vUp, 1.0f),
        float4(input[0].position + fHalfW * vRight + fHalfH * vUp, 1.0f),
        float4(input[0].position - fHalfW * vRight - fHalfH * vUp, 1.0f),
        float4(input[0].position - fHalfW * vRight + fHalfH * vUp, 1.0f)
    };
    float2 UVs[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };
    
    GS_BILLBOARD_OUTPUT output;
    for (int i = 0; i < 4; ++i)
    {
        output.posH = mul(mul(pVertices[i], gmtxView), gmtxProjection);
        output.uv = UVs[i];
        output.textureID = input[0].textureID;
        outStream.Append(output);
    }

}
Texture2D<float4> gtxtBillboardTextures[3] : register(t9);

float4 PSBillboard(GS_BILLBOARD_OUTPUT input) : SV_TARGET
{
    float4 cColor = gtxtBillboardTextures[NonUniformResourceIndex(input.textureID)].Sample(gWrapSamplerState, input.uv);

    return (cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define PT_LAUNCHER 0 //Firework Launcher - launches a PT_SHELL every so many seconds
#define PT_SHELL    1 //Unexploded shell - flies from the origin and explodes into many PT_EMBERXs
#define PT_EMBER1   2 //basic particle - after it's emitted from the shell, it dies
#define PT_EMBER2   3 //after it's emitted, it explodes again into many PT_EMBER1s
#define PT_EMBER3   4 //just a differently colored ember1
#define P_SHELLLIFE 3.0
#define P_EMBER1LIFE 2.5
#define P_EMBER2LIFE 1.5
#define P_EMBER3LIFE 2.0

struct VS_PARTICAL_INPUT
{
    float3 position : POSITION;
    float3 vel : NORMAL;
    float Timer : TIMER;
    uint Type : TYPE;
};

struct VS_PARTICAL_OUTPUT
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float radius : RADIUS;
};

struct GS_PARTICAL_OUTPUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

VS_PARTICAL_OUTPUT VSDrawParticle(VS_PARTICAL_INPUT input)
{
    VS_PARTICAL_OUTPUT output = (VS_PARTICAL_OUTPUT) 0;
    output.pos =input.position;

    output.radius = 10.5;
    
    if (input.Type == PT_LAUNCHER)
    {

        output.color = float4(1, 0.1, 0.1, 1);
        output.radius = 10.0;
    }
    else if (input.Type == PT_SHELL)
    {
        output.color = float4(0.1, 1, 1, 1);
        output.radius = 10.0;
    }
    else if (input.Type == PT_EMBER1)
    {
        output.color = float4(1, 1, 0.1, 1);
        output.color *= (input.Timer / P_EMBER1LIFE);
    }
    else if (input.Type == PT_EMBER2)
    {
        output.color = float4(1, 0.1, 1, 1);
    }
    else if (input.Type == PT_EMBER3)
    {
        output.color = float4(1, 0.1, 0.1, 1);
        output.color *= (input.Timer / P_EMBER3LIFE);
    }
    
    return output;
}

VS_PARTICAL_INPUT VSAdvanceParticle(VS_PARTICAL_INPUT input)
{
    return input;
}

void GSGenericHandler(VS_PARTICAL_INPUT input, inout PointStream<VS_PARTICAL_INPUT> ParticleOutputStream)
{
    input.position += input.vel * gfElapsedTime;
    input.vel += 0.1;
    input.Timer -= gfElapsedTime;
    ParticleOutputStream.Append(input);
}

void GSLauncherHandler(VS_PARTICAL_INPUT input, inout PointStream<VS_PARTICAL_INPUT> ParticleOutputStream)
{
    if (input.Timer <= 0)
    {
        int randVarx = (((gfCurrentTime * 1000) + 1) % 360) - 360;
        int randVary = (((gfCurrentTime * 10000) + 1) % 180) - 180;
        int randVarz = (((gfCurrentTime * 100000) + 1) % 360) - 360;
        float3 vRandom = normalize(float3(randVarx, randVary, randVarz));
        
        //time to emit a new SHELL
        VS_PARTICAL_INPUT output;
        output.position = input.position + input.vel * gfElapsedTime;
        output.vel = input.vel + vRandom * 8.0;
        output.Timer = vRandom.y * 0.5;
        output.Type = PT_SHELL;
        ParticleOutputStream.Append(output);
        
        //reset our timer
        input.Timer = 1.0 + vRandom.x * 0.4;
    }
    else
    {
        input.Timer -= gfElapsedTime;
    }
    
    //emit ourselves to keep us alive
    ParticleOutputStream.Append(input);
    
}

void GSShellHandler(VS_PARTICAL_INPUT input, inout PointStream<VS_PARTICAL_INPUT> ParticleOutputStream)
{
    if (input.Timer <= 0)
    {
        VS_PARTICAL_INPUT output;
        float3 vRandom = float3(0, 0, 0);
        
        int g_iNumEmber1s = 20;
        float g_fMaxEmber2s = 5.0;
        //time to emit a series of new Ember1s  
   
        
        for (int i = 0; i < g_iNumEmber1s; i++)
        {
            int randVarx = (((gfCurrentTime * i*100000) + 1) % 720) - 360;
            int randVary = (((gfCurrentTime * i*10000) + 1) % 360) - 180;
            int randVarz = (((gfCurrentTime * i*1000) + 1) % 720) - 360;
            vRandom = normalize(float3(randVarx, randVary, randVarz));

            output.position = input.position + input.vel * gfElapsedTime;
            output.vel = input.vel + vRandom * 15.0;
            output.Timer = P_EMBER1LIFE + randVary%3;
            output.Type = PT_EMBER1;
            ParticleOutputStream.Append(output);
        }
       
        
    }
    else
    {
        GSGenericHandler(input, ParticleOutputStream);
    }
}

void GSEmber1Handler(VS_PARTICAL_INPUT input, inout PointStream<VS_PARTICAL_INPUT> ParticleOutputStream)
{
    if (input.Timer > 0)
    {
        GSGenericHandler(input, ParticleOutputStream);
    }
}

[maxvertexcount(100)]
void GSAdvanceParticles(point VS_PARTICAL_INPUT input[1], inout PointStream<VS_PARTICAL_INPUT> ParticleOutputStream)
{
    if (input[0].Type == PT_LAUNCHER)
        GSLauncherHandler(input[0], ParticleOutputStream);
    else if (input[0].Type == PT_SHELL)
        GSShellHandler(input[0], ParticleOutputStream);
    else if (input[0].Type == PT_EMBER1 ||
              input[0].Type == PT_EMBER3 || input[0].Type == PT_EMBER2)
        GSEmber1Handler(input[0], ParticleOutputStream);
}

[maxvertexcount(4)]
void GSDrawParticles(point VS_PARTICAL_OUTPUT input[1], inout TriangleStream<GS_PARTICAL_OUTPUT> SpriteStream)
{
    float3 vUp = float3(0.0f, 1.0f, 0.0f);
    float3 vLook = gvCameraPosition.xyz - input[0].pos;
    vLook = normalize(vLook);
    float3 vRight = cross(vUp, vLook);
    float fHalfW = input[0].radius;
    
    float4 pVertices[4] =
    {
    
        float4(input[0].pos + fHalfW * vRight - fHalfW * vUp, 1.0f),
        float4(input[0].pos + fHalfW * vRight + fHalfW * vUp, 1.0f),
        float4(input[0].pos - fHalfW * vRight - fHalfW * vUp, 1.0f),
        float4(input[0].pos - fHalfW * vRight + fHalfW * vUp, 1.0f)
    };
    //uint frame = (1 / input[0].color.a)*10;
    float2 UVs[4] = { float2(0.0f, 0.2f), float2(0.0f, 0.0f), float2(0.16f, 0.2f), float2(0.16f, 0.0f) };
    
    GS_PARTICAL_OUTPUT output;
    for (int i = 0; i < 4; ++i)
    {
        output.posH = mul(mul(pVertices[i], gmtxView), gmtxProjection);
        output.uv = UVs[i];
        output.color = input[0].color; // 수정
        SpriteStream.Append(output);
    }
}
Texture2D<float4> gSmokeTexture : register(t7);

float4 PSDrawParticles(GS_PARTICAL_OUTPUT input) : SV_Target
{
    float4 cColor = gSmokeTexture.Sample(gWrapSamplerState, input.uv);
    cColor.a *= input.color.a;
    return cColor;
}