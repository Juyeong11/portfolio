//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "ComputeShader.h"
#include "Player.h"

class CScene {
public:
	CScene();
	virtual ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void ReleaseObjects() = 0;

	virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice) = 0;
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList* pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void ReleaseShaderVariables() = 0;

	bool ProcessInput(UCHAR* pKeysBuffer);
	virtual void AnimateObjects(float fTimeElapsed) = 0;

	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList) = 0;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, ID3D12Resource* swapchainbuffer = NULL) = 0;

	virtual void ReleaseUploadBuffers() = 0;

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList) {};

	CPlayer* m_pPlayer = NULL;

	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
};

class CSceneOne :public CScene
{
public:
	CSceneOne();
	~CSceneOne();

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList* pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void AnimateObjects(float fTimeElapsed);

	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL,ID3D12Resource* swapchainbuffer = NULL);

	void ReleaseUploadBuffers();

	void CheckObjectBulletCollisions();


	CHeightMapTerrain* GetTerrain() { return(m_pTerrain); }

	ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetComputeRootSignature() { return(m_pd3dComputeRootSignature); }
	void SetComputeRootSignature(ID3D12GraphicsCommandList* pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dComputeRootSignature); }

	ID3D12RootSignature* m_pd3dComputeRootSignature = NULL;
public:
	CShader** m_ppShaders = NULL;
	int			m_nShaders = 0;


	CBlurShader* m_pComputeShader = NULL;

	int nParticleStart = 0;
	CHeightMapTerrain* m_pTerrain = NULL;

	CSkyBox* m_pSkyBox = NULL;
};


struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
};

struct MATERIAL
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

class CSceneTwo : public CScene
{
public:
	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUHandle);
	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {};
	virtual void ReleaseObjects();

	void BuildLightsAndMaterials();

	virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList* pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); }


	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void AnimateObjects(float fTimeElapsed);

	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL, ID3D12Resource* swapchainbuffer = NULL);

	virtual void ReleaseUploadBuffers();

	virtual void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);

	CShader** pObject = nullptr;


	int nShaders = 0;

	LIGHTS* m_pLights = NULL;
	float	m_fLightRotationAngle = 0.0f;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;

	MATERIALS* m_pMaterials = NULL;

	ID3D12Resource* m_pd3dcbMaterials = NULL;
	MATERIAL* m_pcbMappedMaterials = NULL;


	CDepthRenderShader* m_pDepthWriteShader = NULL;
	CShadowMapShader* m_pShadowShader = NULL;
	CTextureToViewportShader* m_pShadowMapToViewport = NULL;
};