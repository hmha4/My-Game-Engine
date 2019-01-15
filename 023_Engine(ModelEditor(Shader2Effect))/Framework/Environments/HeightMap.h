#pragma once

class HeightMap
{
public:
	HeightMap(UINT width, UINT height, float maxHeight);
	~HeightMap();

	void Load(wstring file);

	UINT Width() { return width; }
	UINT Height() { return height; }
	float MaxHeight() { return maxHeight; }

	void Data(UINT row, UINT col, float data);
	float Data(UINT row, UINT col);

	bool InBounds(int row, int col);

	void Smooth();
	float Average(UINT row, UINT col);

	ID3D11ShaderResourceView* BuildSRV();

private:
	float * datas;
	UINT width, height;

	float maxHeight;
};