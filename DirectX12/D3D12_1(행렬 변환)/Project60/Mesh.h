//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMesh
{
public:
	
	CMesh() {
	}
	//CMesh(const CMesh& rhs);
    virtual ~CMesh() { }

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual void ReleaseUploadBuffers() { }


protected:
	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nOffset = 0;

	UINT							m_nType = 0;

public:
	UINT GetType() { return(m_nType); }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet) { }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define VERTEXT_POSITION			0x01
#define VERTEXT_COLOR				0x02
#define VERTEXT_NORMAL				0x04

class CMeshLoadInfo
{
public:
	CMeshLoadInfo() { }
	~CMeshLoadInfo();

public:
	char							m_pstrMeshName[256] = { 0 };

	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	int								m_nVertices = 0;
	XMFLOAT3						*m_pxmf3Positions = NULL;
	XMFLOAT4						*m_pxmf4Colors = NULL;
	XMFLOAT3						*m_pxmf3Normals = NULL;

	int								m_nIndices = 0;
	UINT							*m_pnIndices = NULL;

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = NULL;
	UINT							**m_ppnSubSetIndices = NULL;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CMeshFromFile : public CMesh
{
public:
	CMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CMeshLoadInfo *pMeshInfo);
	//CMeshFromFile(const CMeshFromFile& rhs);
	virtual ~CMeshFromFile();

public:
	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource					*m_pd3dPositionBuffer = NULL;
	ID3D12Resource					*m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = NULL;

	ID3D12Resource					**m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource					**m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW			*m_pd3dSubSetIndexBufferViews = NULL;

public:
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//������ ǥ���ϱ� ���� Ŭ������ �����Ѵ�. 
class CVertex
{
protected:
	//������ ��ġ �����̴�(��� ������ �ּ��� ��ġ ���͸� ������ �Ѵ�). 
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {  };
};


class CCubeMesh : public CMesh
{
private:
	ID3D12Resource* m_pd3dVertexBuffer;
	ID3D12Resource* m_pd3dVertexUploadBuffer;
	ID3D12Resource* m_pd3dIndexBuffer;
	ID3D12Resource* m_pd3dIndexUploadBuffer;

	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;

	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	UINT m_nIndices = 0;
	UINT m_nStride = 0;


public:
	CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMesh();

	virtual void ReleaseUploadBuffers() {
		m_pd3dVertexUploadBuffer->Release(); m_pd3dIndexUploadBuffer->Release(); m_pd3dNormalUploadBuffer->Release();
	}
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};

class CHeightMapImage
{
private:
	//���� �� �̹��� �ȼ�(8bit)���� ������ �迭�̴�
	BYTE* m_pHeightMapPixels;
	//���� �� �̹����� ���ο� ���� ũ���̴�.
	int m_nWidth;
	int m_nLength;
	//���� �� �̹����� ������ �� �� Ȯ���Ͽ� ����� ���ΰ��� ��Ÿ���� ������ �����̴�.
	XMFLOAT3 m_xmf3Scale;
public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage();

	//���� �� �̹������� (x, z)��ġ�� �ȼ� ���� ����� ������ ���̸� ��ȯ�Ѵ�.
	float GetHeight(float x, float z);
	//���� �� �̹������� (x, z)��ġ�� ���� ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 GetHeightMapNormal(int x, int z);

	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	BYTE* GetHeightMapPixels() { return m_pHeightMapPixels; }
	int GetHeightMapWidth() { return m_nWidth; }
	int GetHeightMapLength() { return m_nLength; }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	ID3D12Resource* m_pd3dVertexBuffer;
	ID3D12Resource* m_pd3dVertexUploadBuffer;
	ID3D12Resource* m_pd3dIndexBuffer;
	ID3D12Resource* m_pd3dIndexUploadBuffer;

	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;

	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	UINT m_nIndices = 0;
	UINT m_nStride = 0;

	//������ ũ�� ����x ����y �̴�.
	int m_nWidth;
	int m_nLength;

	//������ ������ �����̴�. ���� ���� �޽��� �� ������ x��ǥ, y��ǥ, z��ǥ�� ��������  x��ǥ, y��ǥ, z��ǥ�� ���� ���� ������.
	//��, ���� ������ x�� ������ ������ 1�� �ƴ϶� ������ ������ x��ǥ�� �ȴ�.
	//�̷��� �ϸ� ���� ���ڸ� ����ϴ���� ū ũ���� ���ڸ� ���� �� �� �ִ�.
	XMFLOAT3 m_xmf3Scale;

public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
		, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		void* pContext = nullptr);
	virtual ~CHeightMapGridMesh();

	virtual void ReleaseUploadBuffers() {
		m_pd3dVertexUploadBuffer->Release(); m_pd3dIndexUploadBuffer->Release(); m_pd3dNormalUploadBuffer->Release();
	}

	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	int GetWidth() { return m_nWidth; }
	int GetLength() { return m_nLength; }


	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
	virtual float OnGetHeight(int x, int z, void* pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void* pContext);
	virtual XMFLOAT3 OnGetAverageNormal(int x, int z, void* pContext);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CMeshIlluminatedFromFile : public CMeshFromFile
{
public:
	CMeshIlluminatedFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CMeshLoadInfo *pMeshInfo);
	//CMeshIlluminatedFromFile(const CMeshIlluminatedFromFile& rhs);
	virtual ~CMeshIlluminatedFromFile();

	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource					*m_pd3dNormalBuffer = NULL;
	ID3D12Resource					*m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

public:

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);
};