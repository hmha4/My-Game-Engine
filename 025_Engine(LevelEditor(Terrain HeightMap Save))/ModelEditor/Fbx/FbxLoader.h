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

	//   ���͸��� ����
	void ExportMaterial(wstring saveFolder = L"", wstring fileName = L"");
	//   �޽� ����
	void ExportMesh(wstring saveFolder = L"", wstring fileName = L"");

	//   �ִϸ��̼� ����
	void GetClipList(vector<wstring>* list);
	void ExportAnimation(UINT clipNumber, wstring saveFolder = L"", wstring fileName = L"");
	void ExportAnimation(wstring clipName, wstring saveFolder = L"", wstring fileName = L"");

private:
	//   Fbx ���� ���� ����
	void ReadMaterial();
	//   XML �������� ���� ���� ����
	void WriteMaterial(wstring saveFolder, wstring fileName);
	//   XML ���͸��� ���� �Է�
	void WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color);
	//   �ؽ��� ������ ���� ������ ������ ����
	void CopyTextureFile(OUT string& textureFile, wstring& saveFolder);

	//   bone(node) ���� ����
	//   Root ���� ���� Ʈ�� ���·� ����Ǿ� ����
	//   �׷��� ���ȣ���� �ؾ��Ѵ�.
	void ReadBoneData(FbxNode * node, int index, int parent);

	UINT GetBoneIndexByName(string name);
	//   �޽� ������ ����(���, �� �޽��� �Ҽӵ� bone)
	void ReadMeshData(FbxNode * node, int parentBone);

	//   ���߿� ��Ű�� �Ҷ� �� �Լ�
	void ReadSkinData();
	void WriteMesh(wstring saveFolder, wstring fileName);

	struct FbxClip* ReadAnimationData(UINT index);
	struct FbxClip* ReadAnimationData(wstring name);
	void ReadKeyframeData(struct FbxClip* clip, FbxNode* node, int start, int stop, int index, int parent);

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
