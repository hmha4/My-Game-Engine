#pragma once

namespace Xml
{
	class XMLDocument;
	class XMLElement;
}

namespace Fbx
{
	class Exporter
	{
	public:
		Exporter(wstring file);
		~Exporter();

		//	���͸��� ����
		void ExportMaterial(wstring saveFolder, wstring fileName);
		//	�޽� ����
		void ExportMesh(wstring saveFolder, wstring fileName);
		//	�ִϸ��̼� ����
		void GetClipCount(vector<wstring>* list);
		void ExportAnimation(wstring saveFolder, wstring fileName, UINT clipNumber);
		void ExportAnimation(wstring saveFolder, wstring fileName, wstring clipName);

		int DeformerCount() { return deformerCount; }
	private:
		//	Fbx ���� ���� ����
		void ReadMaterial();
		//	XML �������� ���� ���� ����
		void WriteMaterial(wstring saveFolder, wstring fileName);
		//	XML ���͸��� ���� �Է�
		void WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color);
		//	�ؽ��� ������ ���� ������ ������ ����
		void CopyTextureFile(OUT string& textureFile, wstring& saveFolder);

		UINT GetBoneIndexByName(string name);
		struct FbxBoneData* GetBoneByIndex(int index);
		//	bone(node) ���� ����
		//	Root ���� ���� Ʈ�� ���·� ����Ǿ� ����
		//	�׷��� ���ȣ���� �ؾ��Ѵ�.
		void ReadBoneData(FbxNode * node, int index, int parent);
		//	�޽� ������ ����(���, �� �޽��� �Ҽӵ� bone)
		void ReadMeshData(FbxNode * node, int parentBone);
		//	���߿� ��ũ�� �Ҷ� �� �Լ�
		void ReadSkinData();
		void WriteMesh(wstring saveFolder, wstring fileName);

		struct FbxClip* ReadAnimationData(UINT index);
		struct FbxClip* ReadAnimationData(wstring name);
		void ReadKeyframeData(struct FbxClip * clip, FbxNode * node, int start, int stop, int index, int parent);

		void WriteClipData(struct FbxClip *clip, wstring saveFolder, wstring fileName);
	private:
		FbxManager * manager;
		FbxImporter * importer;
		FbxScene * scene;
		FbxIOSettings* ios;
		FbxGeometryConverter * converter;

		vector<struct FbxMaterial *> materials;
		vector<struct FbxBoneData *> boneDatas;
		vector<struct FbxMeshData *> meshDatas;
		
		int deformerCount;
	};
}