#include "stdafx.h"
#include "Shader.h"
#include "DDSTextureLoader12.h"
#include "ComputeShader.h"

CComputeShader::CComputeShader()
{

}
CComputeShader::~CComputeShader()
{
	m_pd3dPipelineState->Release();
}
D3D12_SHADER_BYTECODE CComputeShader::CompileShaderFromFile(WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pd3dErrorBlob = NULL;
	::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, NULL);
	if (pd3dErrorBlob)
	{
		char* pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
		std::wcout << pErrorString;
		pd3dErrorBlob->Release();
	}

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}
void CComputeShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CComputeShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
}

void CComputeShader::ReleaseShaderVariables()
{
}

void CComputeShader::ReleaseUploadBuffers()
{
}

void CComputeShader::OnPrepare(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dPipelineState) pd3dCommandList->SetPipelineState(m_pd3dPipelineState);
}

D3D12_SHADER_BYTECODE CComputeShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob)
{
	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = 0;
	d3dShaderByteCode.pShaderBytecode = NULL;

	return(d3dShaderByteCode);
}

void CComputeShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob);
	d3dPipelineStateDesc.NodeMask = 0;
	d3dPipelineStateDesc.CachedPSO = d3dCachedPipelineState;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CBlurShader::CBlurShader()
{
}

CBlurShader::~CBlurShader()
{

}

void CBlurShader::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	//흠
}

void CBlurShader::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{

}

void CBlurShader::ReleaseShaderVariables()
{

}

void CBlurShader::ReleaseUploadBuffers()
{

}

void CBlurShader::Release()
{
	m_pd3dCbvSrvUavDescriptorHeap->Release();
	mBlurMap0->Release();
	mBlurMap1->Release();
	m_phorzPipelineState->Release();
	m_pvertPipelineState->Release();
}

void CBlurShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature)
{
	ID3DBlob* pd3dComputeShaderBlob = NULL;

	D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob);
	d3dPipelineStateDesc.NodeMask = 0;
	d3dPipelineStateDesc.CachedPSO = d3dCachedPipelineState;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_phorzPipelineState);

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();

	pd3dComputeShaderBlob = NULL;


	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
	d3dPipelineStateDesc.CS = CComputeShader::CompileShaderFromFile(L"ComputeShaders.hlsl", "VertBlurCS", "cs_5_1", &pd3dComputeShaderBlob);
	d3dPipelineStateDesc.NodeMask = 0;
	d3dPipelineStateDesc.CachedPSO = d3dCachedPipelineState;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	hResult = pd3dDevice->CreateComputePipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pvertPipelineState);

	if (pd3dComputeShaderBlob) pd3dComputeShaderBlob->Release();
}

D3D12_SHADER_BYTECODE CBlurShader::CreateComputeShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CComputeShader::CompileShaderFromFile(L"ComputeShaders.hlsl", "HorzBlurCS", "cs_5_1", ppd3dShaderBlob));
}


void CBlurShader::BuildResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	mWidth = FRAME_BUFFER_WIDTH;
	mHeight = FRAME_BUFFER_HEIGHT;
	//mBlurMap리소스 만들어주고
	//리소스 뷰 서술자 힙 만듬
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	pd3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap0));

	pd3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap1));

	//CreateCbvSrvUavDescriptorHeap(pd3dDevice, 0, 2, 2);

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = 4; //SRVs + UAVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dCbvSrvUavDescriptorHeap);
	m_pd3dCbvSrvUavDescriptorHeap->SetName(L"Blur");

	mBlur0CpuSrv = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	mBlur0CpuUav.ptr = mBlur0CpuSrv.ptr + gnCbvSrvUavDescriptorIncrementSize;
	mBlur1CpuSrv.ptr = mBlur0CpuUav.ptr + gnCbvSrvUavDescriptorIncrementSize;
	mBlur1CpuUav.ptr = mBlur1CpuSrv.ptr + gnCbvSrvUavDescriptorIncrementSize;

	mBlur0GpuSrv = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	mBlur0GpuUav.ptr = mBlur0GpuSrv.ptr + gnCbvSrvUavDescriptorIncrementSize;
	mBlur1GpuSrv.ptr = mBlur0GpuUav.ptr + gnCbvSrvUavDescriptorIncrementSize;
	mBlur1GpuUav.ptr = mBlur1GpuSrv.ptr + gnCbvSrvUavDescriptorIncrementSize;


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = mFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	pd3dDevice->CreateShaderResourceView(mBlurMap0, &srvDesc, mBlur0CpuSrv);
	pd3dDevice->CreateUnorderedAccessView(mBlurMap0, nullptr, &uavDesc, mBlur0CpuUav);

	pd3dDevice->CreateShaderResourceView(mBlurMap1, &srvDesc, mBlur1CpuSrv);
	pd3dDevice->CreateUnorderedAccessView(mBlurMap1, nullptr, &uavDesc, mBlur1CpuUav);
}

void CBlurShader::Execute(ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, ID3D12Resource* input, int blurCount)
{
	int blurRadius = nWeight / 2;


	pd3dCommandList->SetComputeRootSignature(pd3dRootSignature);

	pd3dCommandList->SetComputeRoot32BitConstants(2, nWeight, gfWeights, 0);

	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	pd3dCommandList->CopyResource(mBlurMap0, input);

	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1,
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvUavDescriptorHeap);
	for (int i = 0; i < blurCount; ++i)
	{
		//가로
		pd3dCommandList->SetPipelineState(m_phorzPipelineState);
		pd3dCommandList->SetComputeRootDescriptorTable(0, mBlur0GpuSrv);
		pd3dCommandList->SetComputeRootDescriptorTable(1, mBlur1GpuUav);

		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		pd3dCommandList->Dispatch(numGroupsX, mHeight, 1);

		pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0,
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
		pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));

		//세로
		pd3dCommandList->SetPipelineState(m_pvertPipelineState);
		pd3dCommandList->SetComputeRootDescriptorTable(0, mBlur1GpuUav);
		pd3dCommandList->SetComputeRootDescriptorTable(1, mBlur0GpuSrv);

		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		pd3dCommandList->Dispatch(mWidth, numGroupsY, 1);

		pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
		pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1,
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	}

	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	pd3dCommandList->CopyResource(input, mBlurMap0);

	// Transition to PRESENT state.
	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(input,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET));
	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap0,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON));
	pd3dCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBlurMap1,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON));
}