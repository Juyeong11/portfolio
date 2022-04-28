#pragma once
class CComputeShader
{
	//1. 계산 쉐이더 설정
	//2. 입력자원(SRV) 출력자원(UAV)로 생성
	//3. Dispatch
	// 이것만하면 집에 보내주시는건가요.. 
private:
	int m_nReferences = 0;
public:
	void AddRef() { m_nReferences++; };
	void Release() { if (--m_nReferences <= 0) delete this; };
public:
	CComputeShader();
	virtual ~CComputeShader();

	virtual D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext);
	virtual void ReleaseShaderVariables();
	
	
	virtual void ReleaseUploadBuffers();

	virtual void OnPrepare(ID3D12GraphicsCommandList* pd3dCommandList);

	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature* pd3dRootSignature);


public:
	ID3D12PipelineState* m_pd3dPipelineState;
};
const int nWeight = 11;
const float gfWeights[nWeight] = { 0.05f,0.05f,0.1f,0.1f,0.1f,0.2f,0.1f,0.1f,0.1f,0.05f,0.05f };
class CBlurShader : public CComputeShader
{
	//랜더링된 결과를 입력으로 받고
	//흐리기된 결과를 출력으로 하고
	//출력된 결과를 랜더링

	//확인해야되는거
	//랜더링된 결과를 어떤식으로 입력으로 넘기나? 후면버퍼의 내용을 그대로? execute해야 그려질텐데? 

	// 루트 시그니처에 준다고 말했고
	// 그럼 이제 줘야지
public:
	CBlurShader();
	virtual ~CBlurShader();

public:
	virtual D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob** ppd3dShaderBlob);
	
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();
	void Release();

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature);

	void Execute(ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dRootSignature,
		ID3D12Resource* input,
		int blurCount);
	
	
	void BuildResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
public:
	ID3D12DescriptorHeap* m_pd3dCbvSrvUavDescriptorHeap = NULL;
	
	

	D3D12_CPU_DESCRIPTOR_HANDLE mBlur0CpuSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE mBlur0CpuUav;

	D3D12_CPU_DESCRIPTOR_HANDLE mBlur1CpuSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE mBlur1CpuUav;

	D3D12_GPU_DESCRIPTOR_HANDLE mBlur0GpuSrv;
	D3D12_GPU_DESCRIPTOR_HANDLE mBlur0GpuUav;

	D3D12_GPU_DESCRIPTOR_HANDLE mBlur1GpuSrv;
	D3D12_GPU_DESCRIPTOR_HANDLE mBlur1GpuUav;

	ID3D12PipelineState* m_phorzPipelineState;
	ID3D12PipelineState* m_pvertPipelineState;

	const int MaxBlurRadius = 5;


	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	ID3D12Resource* mBlurMap0 = nullptr;
	ID3D12Resource* mBlurMap1 = nullptr;
};