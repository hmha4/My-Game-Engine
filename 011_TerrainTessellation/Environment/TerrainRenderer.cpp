#include "stdafx.h"
#include "TerrainRenderer.h"
#include "Terrain.h"
#include "HeightMap.h"
#include "Viewer\Frustum.h"

const float TerrainRenderer::MaxDistance = 500.0f;
const float TerrainRenderer::MinDistance = 20.0f;
const float TerrainRenderer::MaxTessellation = 6.0f;
const float TerrainRenderer::MinTessellation = 0.0f;

TerrainRenderer::TerrainRenderer(Material * material, Terrain * terrain)
	: material(material), terrain(terrain)
{
	D3DXMatrixIdentity(&world);

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(Buffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &cBuffer);
	assert(SUCCEEDED(hr));

	frustum = new Frustum(1000);
}

TerrainRenderer::~TerrainRenderer()
{
	SAFE_DELETE(material);

	SAFE_RELEASE(quadPatchVB);
	SAFE_RELEASE(quadPatchIB);

	SAFE_DELETE(layMapArray);

	SAFE_RELEASE(blendMapSRV);
	SAFE_RELEASE(heightMapSRV);

	SAFE_DELETE(frustum);

	SAFE_RELEASE(cBuffer);
}

void TerrainRenderer::Initialize()
{
	patchVertexRows = ((terrain->Desc().HeightMapHeight - 1) / Terrain::CellsPerPatch) + 1;
	patchVertexCols = ((terrain->Desc().HeightMapWidth - 1) / Terrain::CellsPerPatch) + 1;

	patchVerticesCount = patchVertexRows * patchVertexCols;
	patchQuadFacesCount = (patchVertexRows - 1) * (patchVertexCols - 1);

	//	To Use Tessellation
	{
		CalcAllPatchBoundsY();

		BuildQuadPatchVB();
		BuildQuadPatchIB();
	}

	CreateBlendMap();

	vector<wstring> textures;
	for (int i = 0; i < 5; i++)
		textures.push_back(terrain->Desc().layerMapFile[i]);
	layMapArray = new TextureArray(textures, 512, 512);
	layMapArraySRV = layMapArray->GetSRV();

	heightMapSRV = terrain->GetHeightMap()->BuildSRV();

	Shader * shader = material->GetShader();
	shader->AsShaderResource("HeightMap")->SetResource(heightMapSRV);
	shader->AsShaderResource("LayerMapArray")->SetResource(layMapArraySRV);
	shader->AsShaderResource("BlendMap")->SetResource(blendMapSRV);

}

void TerrainRenderer::Render()
{
	buffer.FogColor = D3DXCOLOR(1, 1, 1, 1);
	buffer.FogStart = 300.0f;
	buffer.FogRange = 200.0f;
	buffer.MinDistance = MinDistance;
	buffer.MaxDistance = MaxDistance;
	buffer.MinTessellation = MinTessellation;
	buffer.MaxTessellation = MaxTessellation;
	buffer.TexelCellSpaceU = 1.0f / terrain->Desc().HeightMapWidth;
	buffer.TexelCellSpaceV = 1.0f / terrain->Desc().HeightMapHeight;
	buffer.WorldCellSpace = terrain->Desc().CellSpacing;

	D3DXPLANE plane[6];
	frustum->GetPlanes(buffer.WorldFrustumPlanes);

	D3D11_MAPPED_SUBRESOURCE subResource;
	HRESULT hr = D3D::GetDC()->Map
	(
		cBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource
	);
	assert(SUCCEEDED(hr));

	memcpy(subResource.pData, &buffer, sizeof(buffer));
	D3D::GetDC()->Unmap(cBuffer, 0);

	hr = material->GetShader()->AsConstantBuffer("CB_Terrain")->SetConstantBuffer(cBuffer);
	assert(SUCCEEDED(hr));

	UINT stride = sizeof(TerrainCP);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &quadPatchVB, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(quadPatchIB, DXGI_FORMAT_R16_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	material->GetShader()->AsMatrix("World")->SetMatrix(world);
	material->GetShader()->DrawIndexed(0, 0, patchQuadFacesCount * 4);
}

void TerrainRenderer::CalcAllPatchBoundsY()
{
	patchBoundsY.assign(patchQuadFacesCount, D3DXVECTOR2());

	for (UINT row = 0; row < patchVertexRows - 1; row++)
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++)
		{
			CalcPatchBoundsY(row, col);
		}
	}
}

void TerrainRenderer::CalcPatchBoundsY(UINT row, UINT col)
{
	UINT x0 = col * Terrain::CellsPerPatch;
	UINT x1 = (col + 1) * Terrain::CellsPerPatch;

	UINT y0 = row * Terrain::CellsPerPatch;
	UINT y1 = (row + 1) * Terrain::CellsPerPatch;

	float minY = FLT_MAX;
	float maxY = FLT_MIN;

	for (UINT y = y0; y <= y1; y++)
	{
		for (UINT x = x0; x <= x1; x++)
		{
			minY = min(minY, terrain->Data(y, x));
			maxY = max(maxY, terrain->Data(y, x));
		}
	}

	UINT patchID = row * (patchVertexCols - 1) + col;
	patchBoundsY[patchID] = D3DXVECTOR2(minY, maxY);
}

void TerrainRenderer::BuildQuadPatchVB()
{
	TerrainCP* patchVertices = new TerrainCP[patchVerticesCount];
	float halfWidth = 0.5f * (float)terrain->Width();
	float halfDepth = 0.5f * (float)terrain->Depth();

	float patchWidth = (float)terrain->Width() / (float)(patchVertexCols - 1);
	float patchDepth = (float)terrain->Depth() / (float)(patchVertexRows - 1);

	float du = 1.0f / (float)(patchVertexCols - 1);
	float dv = 1.0f / (float)(patchVertexRows - 1);

	for (UINT row = 0; row < patchVertexRows; row++)
	{
		float z = halfDepth - (float)row * patchDepth;
		for (UINT col = 0; col < patchVertexCols; col++)
		{
			float x = -halfWidth + (float)col * patchWidth;
			UINT vertId = row * patchVertexCols + col;

			patchVertices[vertId].Position = D3DXVECTOR3(x, 0, z);
			patchVertices[vertId].Uv = D3DXVECTOR2(col * du, row * dv);
		}
	}

	for (UINT row = 0; row < patchVertexRows - 1; row++)
	{
		for (UINT col = 0; col < patchVertexCols - 1; col++)
		{
			UINT patchID = row * (patchVertexCols - 1) + col;
			UINT vertID = row * patchVertexCols + col;
			patchVertices[vertID].BoundsY = patchBoundsY[patchID];
		}
	}

	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(TerrainCP) * patchVerticesCount;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = patchVertices;

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadPatchVB);
	assert(SUCCEEDED(hr));

	SAFE_DELETE_ARRAY(patchVertices);
}

void TerrainRenderer::BuildQuadPatchIB()
{
	vector<WORD> indices;

	for (WORD row = 0; row < (WORD)patchVertexRows - 1; row++)
	{
		for (WORD col = 0; col < (WORD)patchVertexCols - 1; col++)
		{
			indices.push_back(row * patchVertexCols + col);
			indices.push_back(row * patchVertexCols + col + 1);
			indices.push_back((row + 1) * patchVertexCols + col);
			indices.push_back((row + 1) * patchVertexCols + col + 1);
		}
	}

	D3D11_BUFFER_DESC desc = { 0 };
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(WORD) * indices.size();
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = &indices[0];

	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadPatchIB);
	assert(SUCCEEDED(hr));
}

void TerrainRenderer::CreateBlendMap()
{
	HeightMap * heightMap = terrain->GetHeightMap();
	vector<D3DXCOLOR> colors;
	for (UINT y = 0; y < heightMap->Height(); y++)
	{
		for (UINT x = 0; x < heightMap->Width(); x++)
		{
			float elivation = terrain->Data(y, x);

			D3DXCOLOR color = D3DXCOLOR(0, 0, 0, 0);
			if (elivation > heightMap->MaxHeight() * (0.05f + Math::Random(-0.05f, 0.05f)))
			{
				// dark green grass texture
				color.r = elivation / (heightMap->MaxHeight()) + Math::Random(-0.05f, 0.05f);
			}
			if (elivation > heightMap->MaxHeight() * (0.4f + Math::Random(-0.15f, 0.15f)))
			{
				// stone texture
				color.g = elivation / heightMap->MaxHeight() + Math::Random(-0.05f, 0.05f);
			}
			if (elivation > heightMap->MaxHeight() * (0.75f + Math::Random(-0.1f, 0.1f)))
			{
				// snow texture
				color.a = elivation / heightMap->MaxHeight() + Math::Random(-0.05f, 0.05f);
			}

			colors.push_back(color);
		}
	}

	SmoothBlendMap(colors);
	SmoothBlendMap(colors);

	/*vector<UINT> colors8b(colors.size());
	const float f = 255.0f;

	vector<D3DXCOLOR>::iterator it = colors.begin();
	for (UINT i = 0; it != colors.end(); it++, i++)
		colors8b[i] = ((UINT)((f * it->a)) << 24)
		+ ((UINT)((f * it->b)) << 16)
		+ ((UINT)((f * it->g)) << 8)
		+ ((UINT)((f * it->r)) << 0);*/

	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	ID3D11Texture2D* texture = 0;
	//   Create Blend Texture2D
	{
		UINT _width = terrain->GetHeightMap()->Width();
		UINT _height = terrain->GetHeightMap()->Height();

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = _width;
		desc.Height = _height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &colors[0];
		data.SysMemPitch = _width * sizeof(D3DXCOLOR);
		//data.SysMemSlicePitch = _width * _height * sizeof(UINT);

		HRESULT hr = D3D::GetDevice()->CreateTexture2D(&desc, &data, &texture);
		assert(SUCCEEDED(hr));
	}

	//   Create Shader Resource View (To . blendMapSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.MipLevels = 1;

		HRESULT hr = D3D::GetDevice()->CreateShaderResourceView(texture, &desc, &blendMapSRV);
		assert(SUCCEEDED(hr));
	}

	SAFE_RELEASE(texture);
	colors.clear();
	//colors8b.clear();
}

void TerrainRenderer::SmoothBlendMap(vector<D3DXCOLOR>& colors)
{
	HeightMap* heightMap = terrain->GetHeightMap();

	int height = (int)heightMap->Height();
	int width = (int)heightMap->Width();
	float maxHeight = heightMap->MaxHeight();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			D3DXCOLOR sum = colors[x + y * height];
			UINT num = 1;
			for (int y1 = y - 1; y1 < y + 2; y1++)
			{
				for (int x1 = x - 1; x1 < x + 1; x1++)
				{
					if (heightMap->InBounds(y1, x1) == false)
						continue;
					sum += colors[x1 + y1 * height];
					num++;
				} // for(x1)
			} // for(y1)

			colors[x + y * height] = sum / (float)num;
		} // for(x)
	} // for(y)
}
