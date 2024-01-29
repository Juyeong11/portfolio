//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;
	m_pd3dIndexUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}
//인스턴싱을 위한 Render함수

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstances)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, nInstances, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstances, m_nOffset, 0);
	}
}
void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView)
{
	//정점 버퍼 뷰와 인스턴싱 버퍼 뷰를 입력-조립 단계에 설정한다.
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[] = { m_d3dVertexBufferView,d3dInstancingBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, _countof(pVertexBufferViews), pVertexBufferViews);

	Render(pd3dCommandList, nInstances);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMeshIlluminated::CMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CMeshIlluminated::~CMeshIlluminated()
{
}

void CMeshIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices)
{
	int nPrimitives = nVertices / 3;
	UINT nIndex0, nIndex1, nIndex2;
	for (int i = 0; i < nPrimitives; i++)
	{
		nIndex0 = i * 3 + 0;
		nIndex1 = i * 3 + 1;
		nIndex2 = i * 3 + 2;
		XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
		XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
		pxmf3Normals[nIndex0] = pxmf3Normals[nIndex1] = pxmf3Normals[nIndex2] = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
	}
}

void CMeshIlluminated::CalculateTriangleListVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices / 3) : (nVertices / 3);
	XMFLOAT3 xmf3SumOfNormal, xmf3Edge01, xmf3Edge02, xmf3Normal;
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = pnIndices[i * 3 + 0];
			nIndex1 = pnIndices[i * 3 + 1];
			nIndex2 = pnIndices[i * 3 + 2];
			if (pnIndices && ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j)))
			{
				xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
				xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
				xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, false);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}

void CMeshIlluminated::CalculateTriangleStripVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices - 2) : (nVertices - 2);
	XMFLOAT3 xmf3SumOfNormal(0.0f, 0.0f, 0.0f);
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = ((i % 2) == 0) ? (i + 0) : (i + 1);
			if (pnIndices)nIndex0 = pnIndices[nIndex0];
			nIndex1 = ((i % 2) == 0) ? (i + 1) : (i + 0);
			if (pnIndices)nIndex1 = pnIndices[nIndex1];
			nIndex2 = (pnIndices) ? pnIndices[i + 2] : (i + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				XMFLOAT3 xmf3Edge01 = Vector3::Subtract(pxmf3Positions[nIndex1], pxmf3Positions[nIndex0]);
				XMFLOAT3 xmf3Edge02 = Vector3::Subtract(pxmf3Positions[nIndex2], pxmf3Positions[nIndex0]);
				XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge01, xmf3Edge02, true);
				xmf3SumOfNormal = Vector3::Add(xmf3SumOfNormal, xmf3Normal);
			}
		}
		pxmf3Normals[j] = Vector3::Normalize(xmf3SumOfNormal);
	}
}
void CMeshIlluminated::CalculateVertexNormals(XMFLOAT3* pxmf3Normals, XMFLOAT3* pxmf3Positions, int nVertices, UINT* pnIndices, int nIndices)
{
	switch (m_d3dPrimitiveTopology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (pnIndices)
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		else
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices);
		break;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		CalculateTriangleStripVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//
CCubeMeshIlluminated::CCubeMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_nStride = sizeof(CIlluminatedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	XMFLOAT3 pxmf3Positions[36];
	int i = 0;
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);

	pxmf3Positions[i++] = XMFLOAT3(-fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(-fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);

	pxmf3Positions[i++] = XMFLOAT3(+fx, +fy, -fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, +fz);
	pxmf3Positions[i++] = XMFLOAT3(+fx, -fy, -fz);

	XMFLOAT3 pxmf3Normals[36];
	CalculateVertexNormals(pxmf3Normals, pxmf3Positions, m_nVertices, NULL, 0);

	CIlluminatedVertex pVertices[36];
	for (int i = 0; i < 36; i++) pVertices[i] = CIlluminatedVertex(pxmf3Positions[i], pxmf3Normals[i]);

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;


}

CCubeMeshIlluminated::~CCubeMeshIlluminated()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlaneMeshIlluminated::CPlaneMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(CIlluminatedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CIlluminatedVertex pVertices[6];

	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	if (fWidth == 0.0f)
	{
		if (fxPosition > 0.0f)
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT3(-1.0f, 0.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT3(+1.0f, 0.0f, 0.0f));
		}
	}
	else if (fHeight == 0.0f)
	{
		if (fyPosition > 0.0f)
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT3(0.0f, -1.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT3(0.0f, +1.0f, 0.0f));
		}
	}
	else if (fDepth == 0.0f)
	{
		if (fzPosition > 0.0f)
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, -1.0f));
		}
		else
		{
			pVertices[0] = CIlluminatedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[1] = CIlluminatedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[2] = CIlluminatedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[3] = CIlluminatedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[4] = CIlluminatedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
			pVertices[5] = CIlluminatedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT3(0.0f, 0.0f, +1.0f));
		}
	}

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CPlaneMeshIlluminated::~CPlaneMeshIlluminated()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGridMeshIlluminated::CGridMeshIlluminated(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float width, float depth, unsigned int m, unsigned int n) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	m_nVertices = m*n;
	m_nStride = sizeof(CNormalTexturedVertex);
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CNormalTexturedVertex *pVertices = new CNormalTexturedVertex[m_nVertices];

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);
	
	for (size_t i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (size_t j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			pVertices[i * n + j].m_xmf3Position = XMFLOAT3(x, 0.0f, z);
			pVertices[i * n + j].m_xmf3Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			pVertices[i * n + j].m_xmf3TangentU = XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			pVertices[i * n + j].m_xmf2TexCoord.x = j * du;
			pVertices[i * n + j].m_xmf2TexCoord.y = i * dv;
		}
	}

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	delete[] pVertices;

	int faceCount = (m - 1) * (n - 1) * 2;
	int* indices = new int[faceCount * 3];
	m_nIndices = faceCount * 3;
	// Iterate over each quad and compute indices.
	int k = 0;
	for (unsigned int i = 0; i < m - 1; ++i)
	{
		for (unsigned int j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, indices, sizeof(int) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;// 확인 
	m_d3dIndexBufferView.SizeInBytes = m_nIndices * sizeof(int);

	delete[] indices;
}

CGridMeshIlluminated::~CGridMeshIlluminated()
{
}
//////////////////////////////////////////////////////////////////////////////////
//
CVertex_GS::CVertex_GS(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::vector<CTreeVertex>& PointArray) :CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = static_cast<UINT>(PointArray.size());

	m_nStride = sizeof(CTreeVertex);


	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, PointArray.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CVertex_GS::~CVertex_GS()
{
}

void CVertex_GS::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);

	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}

//////////////////////////////////////////////////////////////////////////////////
//

CParticle::CParticle(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, D3D12_CPU_DESCRIPTOR_HANDLE uavStart) :CMesh(pd3dDevice, pd3dCommandList)
{

	m_nVertices = MAXPARTICLES;

	m_nStride = sizeof(CParticleVertex);

	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	CParticleVertex* StartVertex = new CParticleVertex(XMFLOAT3{ 1048.f,170.f,1028.0f }, XMFLOAT3{ 0.f,1.f,0.f },0.f, 0);

	//시작 버퍼 -> 나중에 파티클 출력이 끝나고 이 버퍼의 값으로 stream버퍼의 내용을 복사한다.
	m_pd3dStartBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, StartVertex, m_nStride * MAXPARTICLES, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dStartUploadBuffer);

	m_d3dStartBufferView.BufferLocation = m_pd3dStartBuffer->GetGPUVirtualAddress();
	m_d3dStartBufferView.SizeInBytes = m_nStride * MAXPARTICLES;
	m_d3dStartBufferView.StrideInBytes = m_nStride;

	m_pd3dDrawBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, StartVertex, m_nStride * MAXPARTICLES, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dDrawUploadBuffer);

	m_d3dDrawBufferView.BufferLocation = m_pd3dDrawBuffer->GetGPUVirtualAddress();
	m_d3dDrawBufferView.SizeInBytes = m_nStride * MAXPARTICLES;
	m_d3dDrawBufferView.StrideInBytes = m_nStride;


	m_pd3dStreamBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, StartVertex, m_nStride * MAXPARTICLES, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, &m_pd3dStreamUploadBuffer);
	m_pd3dStreamBuffer->SetName(L"streambuffer");
	m_d3dStreamBufferView.BufferLocation = m_pd3dStreamBuffer->GetGPUVirtualAddress();
	m_d3dStreamBufferView.SizeInBytes = m_nStride * MAXPARTICLES;

	UINT64 nBufferFilledSize = 0;
	m_pd3dBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, &nBufferFilledSize, sizeof(UINT64), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_STREAM_OUT, NULL);


	m_pd3dBufferFilledSize->SetName(L"FilledSize");
	m_pd3dUploadBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dUploadBufferFilledSize->Map(0, NULL, (void**)&m_pnUploadBufferFilledSize);
	m_pd3dUploadBufferFilledSize->SetName(L"UploadBufferFilledSize");
	m_pd3dReadBackBufferFilledSize = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, sizeof(UINT64), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
	m_pd3dReadBackBufferFilledSize->Map(0, NULL, (void**)&m_pnReadBackBufferFilledSize);
	*m_pnReadBackBufferFilledSize = m_nStride;
	m_pd3dUploadBufferFilledSize->SetName(L"ReadBackBufferFilledSize");
	m_d3dStreamBufferView.BufferFilledSizeLocation = m_pd3dBufferFilledSize->GetGPUVirtualAddress();// + ((m_d3dStreamBufferView.SizeInBytes + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1));//8의 배수

	delete StartVertex;
}

CParticle::~CParticle()
{

	if (m_pd3dBufferFilledSize) m_pd3dBufferFilledSize->Release();
	if (m_pd3dReadBackBufferFilledSize) m_pd3dReadBackBufferFilledSize->Release();
	if (m_pd3dUploadBufferFilledSize) m_pd3dUploadBufferFilledSize->Release();
	if (m_pd3dDrawBuffer) m_pd3dDrawBuffer->Release();
	if (m_pd3dStartBuffer) m_pd3dStartBuffer->Release();


	if (m_pd3dStreamBuffer) m_pd3dStreamBuffer->Release();

}
void CParticle::ReleaseUploadBuffers()
{

	if (m_pd3dDrawUploadBuffer) m_pd3dDrawUploadBuffer->Release();
	if (m_pd3dStreamUploadBuffer) m_pd3dStreamUploadBuffer->Release();
	if (m_pd3dStartUploadBuffer) m_pd3dStartUploadBuffer->Release();


	m_pd3dDrawUploadBuffer = NULL;
	m_pd3dStreamUploadBuffer = NULL;

};

//////////////////////////////////////////////////////////////////////////////////
//
CCubeMeshDiffused::CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 8;
	m_nStride = sizeof(CDiffusedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	CDiffusedVertex pVertices[8];

	pVertices[0] = CDiffusedVertex(XMFLOAT3(-fx, +fy, -fz), RANDOM_COLOR);
	pVertices[1] = CDiffusedVertex(XMFLOAT3(+fx, +fy, -fz), RANDOM_COLOR);
	pVertices[2] = CDiffusedVertex(XMFLOAT3(+fx, +fy, +fz), RANDOM_COLOR);
	pVertices[3] = CDiffusedVertex(XMFLOAT3(-fx, +fy, +fz), RANDOM_COLOR);
	pVertices[4] = CDiffusedVertex(XMFLOAT3(-fx, -fy, -fz), RANDOM_COLOR);
	pVertices[5] = CDiffusedVertex(XMFLOAT3(+fx, -fy, -fz), RANDOM_COLOR);
	pVertices[6] = CDiffusedVertex(XMFLOAT3(+fx, -fy, +fz), RANDOM_COLOR);
	pVertices[7] = CDiffusedVertex(XMFLOAT3(-fx, -fy, +fz), RANDOM_COLOR);

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_nIndices = 36;
	UINT pnIndices[36];

	pnIndices[0] = 3; pnIndices[1] = 1; pnIndices[2] = 0;
	pnIndices[3] = 2; pnIndices[4] = 1; pnIndices[5] = 3;
	pnIndices[6] = 0; pnIndices[7] = 5; pnIndices[8] = 4;
	pnIndices[9] = 1; pnIndices[10] = 5; pnIndices[11] = 0;
	pnIndices[12] = 3; pnIndices[13] = 4; pnIndices[14] = 7;
	pnIndices[15] = 0; pnIndices[16] = 4; pnIndices[17] = 3;
	pnIndices[18] = 1; pnIndices[19] = 6; pnIndices[20] = 5;
	pnIndices[21] = 2; pnIndices[22] = 6; pnIndices[23] = 1;
	pnIndices[24] = 2; pnIndices[25] = 7; pnIndices[26] = 6;
	pnIndices[27] = 3; pnIndices[28] = 7; pnIndices[29] = 2;
	pnIndices[30] = 6; pnIndices[31] = 4; pnIndices[32] = 5;
	pnIndices[33] = 7; pnIndices[34] = 4; pnIndices[35] = 6;

	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}

CCubeMeshDiffused::~CCubeMeshDiffused()
{
}

//////////////////////////////////////////////////////////////////////////////////
//
CCubeMeshTextured::CCubeMeshTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_nStride = sizeof(CTexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	CTexturedVertex pVertices[36];
	int i = 0;

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, -fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, -fz), XMFLOAT2(1.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, -fz), XMFLOAT2(1.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, -fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, -fz), XMFLOAT2(1.0f, 1.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, -fz), XMFLOAT2(0.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, +fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, +fz), XMFLOAT2(1.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, -fz), XMFLOAT2(1.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, +fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, -fz), XMFLOAT2(1.0f, 1.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, -fz), XMFLOAT2(0.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, +fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, +fz), XMFLOAT2(1.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, +fz), XMFLOAT2(1.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, +fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, +fz), XMFLOAT2(1.0f, 1.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, +fz), XMFLOAT2(0.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, -fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, -fz), XMFLOAT2(1.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, +fz), XMFLOAT2(1.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, -fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, +fz), XMFLOAT2(1.0f, 1.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, +fz), XMFLOAT2(0.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, +fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, -fz), XMFLOAT2(1.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, -fz), XMFLOAT2(1.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, +fy, +fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, -fz), XMFLOAT2(1.0f, 1.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(-fx, -fy, +fz), XMFLOAT2(0.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, -fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, +fz), XMFLOAT2(1.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, +fz), XMFLOAT2(1.0f, 1.0f));

	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, +fy, -fz), XMFLOAT2(0.0f, 0.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, +fz), XMFLOAT2(1.0f, 1.0f));
	pVertices[i++] = CTexturedVertex(XMFLOAT3(+fx, -fy, -fz), XMFLOAT2(0.0f, 1.0f));

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CCubeMeshTextured::~CCubeMeshTextured()
{
}
//////////////////////////////////////////////////////////////////////////////////
//


//////////////////////////////////////////////////////////////////////////////////
//
CAirplaneMeshDiffused::CAirplaneMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, XMFLOAT4 xmf4Color) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 24 * 3;
	m_nStride = sizeof(CDiffusedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	CDiffusedVertex pVertices[24 * 3];

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);
	int i = 0;

	//Upper Plane
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	//Lower Plane
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	//Right Plane
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	//Back/Right Plane
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	//Left Plane
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	//Back/Left Plane
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color, RANDOM_COLOR));

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CAirplaneMeshDiffused::~CAirplaneMeshDiffused()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}
void BernsteinCoeffcient5x5(float t, float fBernstein[5])// 제어점 5개에 대한 베지에 곡선 공식 베른슈타인 기저 함수
{
	float tInv = 1.0f - t;
	fBernstein[0] = tInv * tInv * tInv * tInv;
	fBernstein[1] = 4.0f * t * tInv * tInv * tInv;
	fBernstein[2] = 6.0f * t * t * tInv * tInv;
	fBernstein[3] = 4.0f * t * t * t * tInv;
	fBernstein[4] = t * t * t * t;
}
XMFLOAT3 CHeightMapImage::CubicBezierSum5x5(XMFLOAT3 patch[25], float u[5], float v[5])
{
	XMFLOAT3 f3Sum = XMFLOAT3(0.0f, 0.0f, 0.0f);
	f3Sum = Vector3::ScalarProduct((Vector3::Add(Vector3::Add(Vector3::Add(Vector3::ScalarProduct(patch[0], u[0], false), Vector3::ScalarProduct(patch[1], u[1], false)), Vector3::Add(Vector3::ScalarProduct(patch[2], u[2], false), Vector3::ScalarProduct(patch[3], u[3], false))), Vector3::ScalarProduct(patch[4], u[4], false))), v[0], false);
	f3Sum = Vector3::Add(f3Sum, Vector3::ScalarProduct((Vector3::Add(Vector3::Add(Vector3::Add(Vector3::ScalarProduct(patch[5], u[0], false), Vector3::ScalarProduct(patch[6], u[1], false)), Vector3::Add(Vector3::ScalarProduct(patch[7], u[2], false), Vector3::ScalarProduct(patch[8], u[3], false))), Vector3::ScalarProduct(patch[9], u[4], false))), v[1], false));
	f3Sum = Vector3::Add(f3Sum, Vector3::ScalarProduct((Vector3::Add(Vector3::Add(Vector3::Add(Vector3::ScalarProduct(patch[10], u[0], false), Vector3::ScalarProduct(patch[11], u[1], false)), Vector3::Add(Vector3::ScalarProduct(patch[12], u[2], false), Vector3::ScalarProduct(patch[13], u[3], false))), Vector3::ScalarProduct(patch[14], u[4], false))), v[2], false));
	f3Sum = Vector3::Add(f3Sum, Vector3::ScalarProduct((Vector3::Add(Vector3::Add(Vector3::Add(Vector3::ScalarProduct(patch[15], u[0], false), Vector3::ScalarProduct(patch[16], u[1], false)), Vector3::Add(Vector3::ScalarProduct(patch[17], u[2], false), Vector3::ScalarProduct(patch[18], u[3], false))), Vector3::ScalarProduct(patch[19], u[4], false))), v[3], false));
	f3Sum = Vector3::Add(f3Sum, Vector3::ScalarProduct((Vector3::Add(Vector3::Add(Vector3::Add(Vector3::ScalarProduct(patch[20], u[0], false), Vector3::ScalarProduct(patch[21], u[1], false)), Vector3::Add(Vector3::ScalarProduct(patch[22], u[2], false), Vector3::ScalarProduct(patch[23], u[3], false))), Vector3::ScalarProduct(patch[24], u[4], false))), v[4], false));

	return (f3Sum);
}

float CHeightMapImage::GetHeight(float fx, float fz, int cxQuadsPerBlock, int czQuadsPerBlock)
{

	fx = (fx) / m_xmf3Scale.x;
	fz = (fz) / m_xmf3Scale.z;


	int xStart = static_cast<int>(fx / cxQuadsPerBlock);
	int zStart = static_cast<int>(fz / czQuadsPerBlock);
	xStart *= cxQuadsPerBlock;
	zStart *= czQuadsPerBlock;


	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);


	float u[5], v[5];
	float t1 = (fx - xStart) / 12.f;//
	float t2 = (fz - zStart) / 12.f;

	BernsteinCoeffcient5x5(t1, u);
	BernsteinCoeffcient5x5(t2, v);

	float fheight;
	XMFLOAT3 patch[25];
	int nIncrease = 3;//0 3 6 9 12 -> 5개
	for (int i = 0; i < 5; ++i)
		for (int j = 0; j < 5; ++j) {
			fheight = (float)m_pHeightMapPixels[xStart + i * nIncrease + ((zStart + j * nIncrease) * m_nWidth)];
			patch[i + 5 * j] = XMFLOAT3(static_cast<float>(xStart + i * nIncrease), fheight, static_cast<float>(zStart + j * nIncrease));
		}
	XMFLOAT3 pos = CubicBezierSum5x5(patch, u, v);


	return(pos.y);
}
#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetInterpolatedHeight(int x, int z, float xFractional, float zFractional, bool bReverseQuad)
{
	if ((x < 0) || (z < 0) || (x >= m_nWidth) || (z >= m_nLength)) return(0.0f);

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (zFractional >= xFractional)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (zFractional < (1.0f - xFractional))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - xFractional) + fTopRight * xFractional;
	float fBottomHeight = fBottomLeft * (1 - xFractional) + fBottomRight * xFractional;
	float fHeight = fBottomHeight * (1 - zFractional) + fTopHeight * zFractional;

	return(fHeight);
}

float CHeightMapImage::GetHeight(float fx, float fz, XMFLOAT3 xmf3Scale)
{
	fx /= xmf3Scale.x;
	fz /= xmf3Scale.z;

	int x = (int)fx;
	int z = (int)fz;
	float xFractional = fx - x;
	float zFractional = fz - z;

	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = GetInterpolatedHeight(x, z, xFractional, zFractional, bReverseQuad);

	return(fHeight * xmf3Scale.y);
}


CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 25;;
	//	m_nStride = sizeof(CTexturedVertex);
	m_nStride = sizeof(CDiffused2TexturedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	//	CTexturedVertex *pVertices = new CTexturedVertex[m_nVertices];

		//uv좌표가 있는 정점
	CDiffused2TexturedVertex* pVertices = new CDiffused2TexturedVertex[m_nVertices];

	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;


	int nIncrease = 3; //(Block Size == 9) ? 2, (Block Size == 13) ? 3
	for (int i = 0, z = (zStart + nLength - 1); z >= zStart; z -= nIncrease)
	{
		for (int x = xStart; x < (xStart + nWidth); x += nIncrease, i++)
		{
			float xPosition = x * m_xmf3Scale.x, zPosition = z * m_xmf3Scale.z;
			fHeight = pHeightMapImage->GetHeight(xPosition, zPosition, m_xmf3Scale);
			pVertices[i].m_xmf3Position = XMFLOAT3(xPosition, fHeight, zPosition);
			pVertices[i].m_xmf4Diffuse = Vector4::Add(OnGetColor(int(x), int(z), pContext), xmf4Color);
			pVertices[i].m_xmf2TexCoord0 = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			pVertices[i].m_xmf2TexCoord1 = XMFLOAT2(float(x) / float(m_xmf3Scale.x * 0.5f), float(z) / float(m_xmf3Scale.z * 0.5f));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	//for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	//{
	//	for (int x = xStart; x < (xStart + nWidth); x++, i++)
	//	{
	//		fHeight = OnGetHeight(x, z, pContext);
	//		pVertices[i].m_xmf3Position = XMFLOAT3((x * m_xmf3Scale.x), fHeight, (z * m_xmf3Scale.z));
	//		pVertices[i].m_xmf4Diffuse = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);

	//		//각 정점이 uv좌표를 가지도록 지형의 높낮이, 지형 텍스쳐와 지형의 크기 고려하기 위해 디테일 텍스쳐를 위한 uv좌표
	//		pVertices[i].m_xmf2TexCoord0 = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
	//		pVertices[i].m_xmf2TexCoord1 = XMFLOAT2(float(x) / float(m_xmf3Scale.x * 0.5f), float(z) / float(m_xmf3Scale.z * 0.5f));
	//		if (fHeight < fMinHeight) fMinHeight = fHeight;
	//		if (fHeight > fMaxHeight) fMaxHeight = fHeight;
	//	}
	//}

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	delete[] pVertices;


}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexturedRectMesh::CTexturedRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(CTexturedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CTexturedVertex pVertices[6];

	float fx = (fWidth * 0.5f) + fxPosition, fy = (fHeight * 0.5f) + fyPosition, fz = (fDepth * 0.5f) + fzPosition;

	if (fWidth == 0.0f)
	{
		if (fxPosition > 0.0f)
		{
			pVertices[0] = CTexturedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT2(1.0f, 0.0f));
			pVertices[1] = CTexturedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT2(1.0f, 1.0f));
			pVertices[2] = CTexturedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[3] = CTexturedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[4] = CTexturedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT2(0.0f, 0.0f));
			pVertices[5] = CTexturedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT2(1.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CTexturedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT2(1.0f, 0.0f));
			pVertices[1] = CTexturedVertex(XMFLOAT3(fx, -fy, +fz), XMFLOAT2(1.0f, 1.0f));
			pVertices[2] = CTexturedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[3] = CTexturedVertex(XMFLOAT3(fx, -fy, -fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[4] = CTexturedVertex(XMFLOAT3(fx, +fy, -fz), XMFLOAT2(0.0f, 0.0f));
			pVertices[5] = CTexturedVertex(XMFLOAT3(fx, +fy, +fz), XMFLOAT2(1.0f, 0.0f));
		}
	}
	else if (fHeight == 0.0f)
	{
		if (fyPosition > 0.0f)
		{
			pVertices[0] = CTexturedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT2(1.0f, 0.0f));
			pVertices[1] = CTexturedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT2(1.0f, 1.0f));
			pVertices[2] = CTexturedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[3] = CTexturedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[4] = CTexturedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT2(0.0f, 0.0f));
			pVertices[5] = CTexturedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT2(1.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CTexturedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT2(1.0f, 0.0f));
			pVertices[1] = CTexturedVertex(XMFLOAT3(+fx, fy, -fz), XMFLOAT2(1.0f, 1.0f));
			pVertices[2] = CTexturedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[3] = CTexturedVertex(XMFLOAT3(-fx, fy, -fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[4] = CTexturedVertex(XMFLOAT3(-fx, fy, +fz), XMFLOAT2(0.0f, 0.0f));
			pVertices[5] = CTexturedVertex(XMFLOAT3(+fx, fy, +fz), XMFLOAT2(1.0f, 0.0f));
		}
	}
	else if (fDepth == 0.0f)
	{
		if (fzPosition > 0.0f)
		{
			pVertices[0] = CTexturedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
			pVertices[1] = CTexturedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT2(1.0f, 1.0f));
			pVertices[2] = CTexturedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[3] = CTexturedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[4] = CTexturedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT2(0.0f, 0.0f));
			pVertices[5] = CTexturedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
		}
		else
		{
			pVertices[0] = CTexturedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
			pVertices[1] = CTexturedVertex(XMFLOAT3(-fx, -fy, fz), XMFLOAT2(1.0f, 1.0f));
			pVertices[2] = CTexturedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[3] = CTexturedVertex(XMFLOAT3(+fx, -fy, fz), XMFLOAT2(0.0f, 1.0f));
			pVertices[4] = CTexturedVertex(XMFLOAT3(+fx, +fy, fz), XMFLOAT2(0.0f, 0.0f));
			pVertices[5] = CTexturedVertex(XMFLOAT3(-fx, +fy, fz), XMFLOAT2(1.0f, 0.0f));
		}
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CTexturedRectMesh::~CTexturedRectMesh()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRawFormatImage::CRawFormatImage(LPCTSTR pFileName, int nWidth, int nLength, bool bFlipY)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int arrSize = m_nWidth * m_nLength;
	BYTE* pRawImagePixels = new BYTE[arrSize];
	ZeroMemory(pRawImagePixels, arrSize);

	std::ifstream in(pFileName, std::ios::binary);
	if (!in) {
		return;
	}
	tagRGBQUAD ColorTable[256];
	in.seekg(54);//비트맵 헤더에서 색상 테이블 시작 부분
	in.read((char*)&ColorTable, sizeof(ColorTable));

	in.seekg(10);//비트맵 헤더에서 픽셀 인덱스 데이터 시작 부분
	int pos;
	in.read((char*)&pos, sizeof(int));
	in.seekg(pos);

	char index;
	for (int i = 0; i < arrSize; ++i)
	{
		in.read(&index, sizeof(char));
		char c = 0;
		c |= 0xe0 & ColorTable[(BYTE)index].rgbBlue;
		c |= 0x1c & ColorTable[(BYTE)index].rgbGreen >> 3;
		c |= 0x03 & ColorTable[(BYTE)index].rgbRed >> 6;
		//std::cout << (int)(BYTE)index << std::endl;
		pRawImagePixels[i] = static_cast<char>(c);
	}
	m_pRawImagePixels = pRawImagePixels;
}

CRawFormatImage::~CRawFormatImage()
{
	if (m_pRawImagePixels) delete[] m_pRawImagePixels;
	m_pRawImagePixels = NULL;
}

CBillboardMesh::CBillboardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(CTexturedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CTexturedVertex pVertices[6];

	float fx = (fWidth * 0.5f), fy = (fHeight * 0.5f);

	pVertices[0] = CTexturedVertex(XMFLOAT3(+fx, -fy, 0.0f), XMFLOAT2(0.0f, 1.0f));
	pVertices[1] = CTexturedVertex(XMFLOAT3(+fx, +fy, 0.0f), XMFLOAT2(0.0f, 0.0f));
	pVertices[2] = CTexturedVertex(XMFLOAT3(-fx, -fy, 0.0f), XMFLOAT2(1.0f, 1.0f));
	pVertices[3] = CTexturedVertex(XMFLOAT3(-fx, -fy, 0.0f), XMFLOAT2(1.0f, 1.0f));
	pVertices[4] = CTexturedVertex(XMFLOAT3(+fx, +fy, 0.0f), XMFLOAT2(0.0f, 0.0f));
	pVertices[5] = CTexturedVertex(XMFLOAT3(-fx, +fy, 0.0f), XMFLOAT2(1.0f, 0.0f));

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CBillboardMesh::~CBillboardMesh()
{
}

CEffectMesh::CEffectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 6;
	m_nStride = sizeof(CTexturedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	CTexturedVertex pVertices[6];

	float fx = (fWidth * 0.5f), fy = (fHeight * 0.5f);

	pVertices[0] = CTexturedVertex(XMFLOAT3(+fx, -fy, 0.0f), XMFLOAT2(0.0f, 1.0f / 4.f));
	pVertices[1] = CTexturedVertex(XMFLOAT3(+fx, +fy, 0.0f), XMFLOAT2(0.0f, 0.0f));
	pVertices[2] = CTexturedVertex(XMFLOAT3(-fx, -fy, 0.0f), XMFLOAT2(1.0f / 5.f, 1.0f / 4.f));

	pVertices[3] = CTexturedVertex(XMFLOAT3(-fx, -fy, 0.0f), XMFLOAT2(1.0f / 5.f, 1.0f / 4.f));
	pVertices[4] = CTexturedVertex(XMFLOAT3(+fx, +fy, 0.0f), XMFLOAT2(0.0f, 0.0f));
	pVertices[5] = CTexturedVertex(XMFLOAT3(-fx, +fy, 0.0f), XMFLOAT2(1.0f / 5.f, 0.0f));

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CEffectMesh::~CEffectMesh()
{
}


CSkullMesh::CSkullMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMeshIlluminated(pd3dDevice, pd3dCommandList)
{
	std::ifstream fin("Models/skull.txt");

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	
	m_nStride = sizeof(CNormalTexturedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	std::vector<CNormalTexturedVertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].m_xmf3Position.x >> vertices[i].m_xmf3Position.y >> vertices[i].m_xmf3Position.z;
		fin >> vertices[i].m_xmf3Normal.x >> vertices[i].m_xmf3Normal.y >> vertices[i].m_xmf3Normal.z;
		vertices[i].m_xmf2TexCoord = XMFLOAT2{ 0.f,2.f };
		vertices[i].m_xmf3TangentU = XMFLOAT3{ 0.f, 0.f,0.f };
	}
	m_nVertices = vcount;

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, vertices.data(), m_nStride * vcount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * vcount;

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<int> indices(3 * tcount);

	for (size_t i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
	m_nIndices = 3 * tcount;
	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, indices.data(), sizeof(int) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;// 확인 
	m_d3dIndexBufferView.SizeInBytes = m_nIndices * sizeof(int);
}

CSkullMesh::~CSkullMesh()
{
}