#pragma once
class CComputeShader
{
	//1. ��� ���̴� ����
	//2. �Է��ڿ�(SRV) ����ڿ�(UAV)�� ����
	//3. Dispatch
	// �̰͸��ϸ� ���� �����ֽô°ǰ���.. 
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
	//�������� ����� �Է����� �ް�
	//�帮��� ����� ������� �ϰ�
	//��µ� ����� ������

	//Ȯ���ؾߵǴ°�
	//�������� ����� ������� �Է����� �ѱ⳪? �ĸ������ ������ �״��? execute�ؾ� �׷����ٵ�? 

	// ��Ʈ �ñ״�ó�� �شٰ� ���߰�
	// �׷� ���� �����
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