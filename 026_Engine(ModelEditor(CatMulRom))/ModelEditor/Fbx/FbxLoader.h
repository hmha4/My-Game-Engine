#pragma once

namespace Xml
{
	class XMLDocument;
	class XMLElement;
}

class FbxLoader
{
public:
	FbxLoader(wstring file, wstring savefolder, wstring saveName);
	~FbxLoader();

	//	Reading materials and meshes
	void ExportMaterial(wstring saveFolder = L"", wstring fileName = L"");
	void ExportMesh(wstring saveFolder = L"", wstring fileName = L"");

	//	Reading animation clip datas
	void GetClipList(vector<wstring>* list);
	void ExportAnimation(UINT clipNumber, wstring saveFolder = L"", wstring fileName = L"");
	void ExportAnimation(wstring clipName, wstring saveFolder = L"", wstring fileName = L"");

private:
	//	Read Material
	void ReadMaterial();
	//	Write material datas with XML
	void WriteMaterial(wstring saveFolder, wstring fileName);
	//	Write material colors with XML
	void WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color);
	//	Copy texture files to a specified folder
	void CopyTextureFile(OUT string& textureFile, wstring& saveFolder);

	//	Read bone datas
	//	Bones are arranged as a tree form from the root node
	//	So it needs to be recursive
	void ReadBoneData(FbxNode * node, int index, int parent);
	//	Get bone index with its name
	UINT GetBoneIndexByName(string name);
	//	Read mesh datas
	void ReadMeshData(FbxNode * node, int parentBone);

	//	Read skin datas
	void ReadSkinData();
	//	Write mesh datas with XML
	void WriteMesh(wstring saveFolder, wstring fileName);

	//	Read animation datas with its index
	struct FbxClip* ReadAnimationData(UINT index);
	//	Read animation datas with its name
	struct FbxClip* ReadAnimationData(wstring name);
	//	Read key frame datas
	void ReadKeyframeData(struct FbxClip* clip, FbxNode* node, int start, int stop, int index, int parent);
	//	Write clip datas with XML
	void WriteClipData(struct FbxClip *clip, wstring saveFolder, wstring fileName);

private:
	FbxManager * manager;
	FbxImporter * importer;
	FbxScene * scene;
	FbxIOSettings* ios;
	FbxGeometryConverter * converter;

	wstring fbxFile;
	wstring saveFolder;
	wstring saveName;

	vector<struct FbxMaterial *> materials;
	vector<struct FbxBoneData *> boneDatas;
	vector<struct FbxMeshData *> meshDatas;
};
