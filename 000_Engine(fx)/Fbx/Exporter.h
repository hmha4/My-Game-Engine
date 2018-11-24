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

		//	머터리얼 추출
		void ExportMaterial(wstring saveFolder, wstring fileName);
		//	메쉬 추출
		void ExportMesh(wstring saveFolder, wstring fileName);
		//	애니메이션 추출
		void GetClipCount(vector<wstring>* list);
		void ExportAnimation(wstring saveFolder, wstring fileName, UINT clipNumber);
		void ExportAnimation(wstring saveFolder, wstring fileName, wstring clipName);

		int DeformerCount() { return deformerCount; }
	private:
		//	Fbx 파일 정보 추출
		void ReadMaterial();
		//	XML 형식으로 추출 정보 저장
		void WriteMaterial(wstring saveFolder, wstring fileName);
		//	XML 머터리얼 색상 입력
		void WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color);
		//	텍스쳐 파일을 내가 지정한 폴더로 복사
		void CopyTextureFile(OUT string& textureFile, wstring& saveFolder);

		UINT GetBoneIndexByName(string name);
		struct FbxBoneData* GetBoneByIndex(int index);
		//	bone(node) 정보 추출
		//	Root 노드로 부터 트리 형태로 저장되어 있음
		//	그래서 재귀호출을 해야한다.
		void ReadBoneData(FbxNode * node, int index, int parent);
		//	메쉬 데이터 추출(노드, 이 메쉬가 소속될 bone)
		void ReadMeshData(FbxNode * node, int parentBone);
		//	나중에 스크닝 할때 쓸 함수
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