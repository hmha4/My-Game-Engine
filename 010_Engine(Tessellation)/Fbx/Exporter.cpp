#include "stdafx.h"
#include "Exporter.h"
#include "Type.h"
#include "Utility.h"
#include "../Utilities/Xml.h"
#include "../Utilities/BinaryFile.h"

Fbx::Exporter::Exporter(wstring file)
{
	//	매니져 생성
	manager = FbxManager::Create();
	//	씬 생성(트리 형태로 파일의 노드를 가지고 있음)
	scene = FbxScene::Create(manager, "");

	ios = FbxIOSettings::Create(manager, IOSROOT);
	//	FBX 내의 옵션 설정
	//	FBX 내의 원하는 설정의 파일을 압출 풀어 파일형태로 가져옴
	ios->SetBoolProp(IMP_FBX_TEXTURE, true);
	//	매니져에 ios 연결
	manager->SetIOSettings(ios);

	//	임포터 생성
	//	임포터에 매니져 연결
	//	"" : 이름
	importer = FbxImporter::Create(manager, "");

	string sFile = String::ToString(file);
	//	(파일 이름, 포맷번호, IOS)
	//	파일 번호 : -1 - 모든 포맷
	//				0 - FBX
	//				1 - OBJ
	//	...

	//	importer에 대한 초기화
	//	import시킬 파일, 파일 포맷, IOS(없다면, 자동을 디폴트 ios를 만듬)
	bool b = importer->Initialize(sFile.c_str(), -1, ios);
	assert(b == true);

	//	연 파일을 씬에다가 넣음
	//	씬 밑에 루트 노드가 생김
	b = importer->Import(scene);
	assert(b == true);

	//	Mesh, Nurbs, Patches를 지원하고
	//	삼각형화 시키고, Nurbs와 Patches의 변경을 가능하게 해주는 클래스
	converter = new FbxGeometryConverter(manager);

	deformerCount = 0;
}

Fbx::Exporter::~Exporter()
{
	SAFE_DELETE(converter);

	importer->Destroy();
	ios->Destroy();
	scene->Destroy();
	manager->Destroy();
}

void Fbx::Exporter::ExportMaterial(wstring saveFolder, wstring fileName)
{
	ReadMaterial();
	WriteMaterial(saveFolder, fileName);
}

void Fbx::Exporter::ExportMesh(wstring saveFolder, wstring fileName)
{
	////////////////////////////////////////////////
	MyGui::log.AddLog("\nReading Bones and Meshes...\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////
	ReadBoneData(scene->GetRootNode(), -1, -1);
	////////////////////////////////////////////////
	MyGui::log.AddLog("===============================================\n");
	MyGui::log.AddLog("Bone and Mesh Datas [LOAD SUCCEEDED]\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////
	ReadSkinData();
	////////////////////////////////////////////////
	MyGui::log.AddLog("===============================================\n");
	MyGui::log.AddLog("Skin Datas [LOAD SUCCEEDED]\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////
	WriteMesh(saveFolder, fileName);
}

void Fbx::Exporter::GetClipCount(vector<wstring>* list)
{
	list->clear();
	for (int i = 0; i < importer->GetAnimStackCount(); i++)
	{
		FbxTakeInfo *takeInfo = importer->GetTakeInfo(i);

		string name = takeInfo->mName.Buffer();
		list->push_back(String::ToWString(name));
	}
}

void Fbx::Exporter::ExportAnimation(wstring saveFolder, wstring fileName, UINT clipNumber)
{
	////////////////////////////////////////////////
	MyGui::log.AddLog("\nReading Animations...\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////
	FbxClip * clip = ReadAnimationData(clipNumber);
	////////////////////////////////////////////////
	MyGui::log.AddLog("===============================================\n");
	MyGui::log.AddLog("Animation Datas [LOAD SUCCEEDED]\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////
	WriteClipData(clip, saveFolder, fileName);
}

void Fbx::Exporter::ExportAnimation(wstring saveFolder, wstring fileName, wstring clipName)
{
	FbxClip * clip = ReadAnimationData(clipName);
	WriteClipData(clip, saveFolder, fileName);
}


void Fbx::Exporter::ReadMaterial()
{
	////////////////////////////////////////////////
	MyGui::log.AddLog("Reading Materials...\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////

	//	씬에서 머티리얼의 갯수를 받아옴
	//	한 Fbx에 머터리얼의 갯수는 한개 이상이다.
	int count = scene->GetMaterialCount();

	////////////////////////////////////////////////
	MyGui::log.AddLog("Material Count : %d\n", count);
	////////////////////////////////////////////////

	for (int i = 0; i < count; i++)
	{
		//	씬에서 각각의 머터리얼 정보를 받아옴
		FbxSurfaceMaterial * fbxMaterial = scene->GetMaterial(i);
		//	XML 형식으로 저장하기 위해
		//	구조체에 저장
		FbxMaterial * material = new FbxMaterial;
		material->Name = fbxMaterial->GetName();

		////////////////////////////////////////////////
		MyGui::log.AddLog("Material Name : %s\n", material->Name.c_str());
		////////////////////////////////////////////////

		//	FbxSurfaceMaterial이 FbxSurfaceLambert로 다운캐스팅이 가능한지
		if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) == true)
		{
			// 가능하다면 다운캐스팅후 lambert정보를 받아옴
			FbxSurfaceLambert * lambert = (FbxSurfaceLambert*)fbxMaterial;

			//	lambert->Diffuse : Diffuse칼라 값(rgb)
			//	lambert->DiffuseFactor : 강도(a)
			material->Diffuse = Utility::ToColor(lambert->Diffuse, lambert->DiffuseFactor);

			////////////////////////////////////////////////
			MyGui::log.AddLog("Diffuse Color : R[%.2f] G[%.2f] B[%.2f] A[%.2f]\n", material->Diffuse.r, material->Diffuse.g, material->Diffuse.b, material->Diffuse.a);
			////////////////////////////////////////////////
		}

		//	습성 정보
		FbxProperty prop;
		//	현재 Diffuse의 이름으로 습성을 찾아서 받아옴
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		//	Diffuse의 파일경로를 얻어옴
		material->DiffuseFile = Utility::GetTextureFile(prop);

		////////////////////////////////////////////////
		MyGui::log.AddLog("Diffuse Map : %s\n", material->DiffuseFile.c_str());
		////////////////////////////////////////////////

		//	노말 텍스쳐
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
		material->NormalFile = Utility::GetTextureFile(prop);

		////////////////////////////////////////////////
		MyGui::log.AddLog("Normal Map : %s\n", material->NormalFile.c_str());
		////////////////////////////////////////////////

		//	스페큘러 텍스쳐
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		material->SpecularFile = Utility::GetTextureFile(prop);

		////////////////////////////////////////////////
		MyGui::log.AddLog("Specular Map : %s\n", material->SpecularFile.c_str());
		////////////////////////////////////////////////

		materials.push_back(material);
	}

	////////////////////////////////////////////////
	MyGui::log.AddLog("=============================================\n");
	MyGui::log.AddLog("Material Datas [LOAD SUCCEEDED]\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////
}

void Fbx::Exporter::WriteMaterial(wstring saveFolder, wstring fileName)
{
	//	지정한 위치에 폴더를 만듬
	Path::CreateFolder(saveFolder);

	//	모든 노드는 XMLDocument에 연결된다
	//	document가 사라지면 연결된 모든 노드도 같이 사라진다.
	Xml::XMLDocument * document = new Xml::XMLDocument();
	//	Declaration(선언)이 첫 진입점이 된다.
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	//	Materials란 노드를 root 노드로 아래에
	//	Material종류 별로 연결
	Xml::XMLElement * root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (FbxMaterial* material : materials)
	{
		//	root 아래에 연결
		Xml::XMLElement * node = document->NewElement("Material");
		root->LinkEndChild(node);

		Xml::XMLElement * element = NULL;

		//	Material이란 노드 아래에 연결
		element = document->NewElement("Name");
		element->SetText(material->Name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		node->LinkEndChild(element);

		//	Diffuse노드 아래에 색 연결
		WriteXmlColor(element, document, material->Diffuse);

		element = document->NewElement("DiffuseFile");
		CopyTextureFile(material->DiffuseFile, saveFolder);
		element->SetText(material->DiffuseFile.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		CopyTextureFile(material->NormalFile, saveFolder);
		element->SetText(material->NormalFile.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		CopyTextureFile(material->SpecularFile, saveFolder);
		element->SetText(material->SpecularFile.c_str());
		node->LinkEndChild(element);

		//	ReadMaterial에서 할당한 material 해제
		SAFE_DELETE(material);
	}

	//	파일을 지정 경로에 저장
	string file = String::ToString(saveFolder + fileName);
	document->SaveFile(file.c_str());
}

void Fbx::Exporter::WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color)
{
	Xml::XMLElement * newColor = NULL;

	newColor = document->NewElement("R");
	newColor->SetText(color.r);
	element->LinkEndChild(newColor);

	newColor = document->NewElement("G");
	newColor->SetText(color.g);
	element->LinkEndChild(newColor);

	newColor = document->NewElement("B");
	newColor->SetText(color.b);
	element->LinkEndChild(newColor);

	newColor = document->NewElement("B");
	newColor->SetText(color.a);
	element->LinkEndChild(newColor);
}

void Fbx::Exporter::CopyTextureFile(OUT string & textureFile, wstring & saveFolder)
{
	if (textureFile.length() < 1)
		return;

	wstring file = String::ToWString(textureFile);
	wstring fileName = Path::GetFileName(file);

	//	파일이 존재한다면, 경로에 한글 안됨
	if (Path::ExistFile(textureFile) == true)
		//	파일 복사 
		CopyFile(file.c_str(), (saveFolder + fileName).c_str(), FALSE);

	textureFile = String::ToString(fileName);
}

UINT Fbx::Exporter::GetBoneIndexByName(string name)
{
	for (UINT i = 0; i < boneDatas.size(); i++)
	{
		if (boneDatas[i]->Name == name)
			return i;
	}

	return -1;
}

Fbx::FbxBoneData * Fbx::Exporter::GetBoneByIndex(int index)
{
	for (UINT i = 0; i < boneDatas.size(); i++)
	{
		if (boneDatas[i]->Index == index)
			return boneDatas[i];
	}

	return NULL;
}

//	처음 호출 시에는 root노드가 들어감
void Fbx::Exporter::ReadBoneData(FbxNode * node, int index, int parent)
{
	//	FbxNodeAttribute : 현재 노드가 어떤 데이터를 가지고 있는지 결정 해준다.
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	// 데이터가 있다면
	if (attribute != NULL)
	{
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();

		//	밑의 조건이 하나라도 만족하면 b 가 true가 된다.
		bool b = false;
		b |= (nodeType == FbxNodeAttribute::eSkeleton);
		b |= (nodeType == FbxNodeAttribute::eMesh);
		b |= (nodeType == FbxNodeAttribute::eNull);
		b |= (nodeType == FbxNodeAttribute::eMarker);

		if (b == true)
		{
			FbxBoneData * bone = new FbxBoneData();
			bone->Index = index;
			bone->Parent = parent;
			bone->Name = node->GetName();

			////////////////////////////////////////////////
			MyGui::log.AddLog("Index[%d], Parent[%d], Name[%s]\n", index, parent, bone->Name.c_str());
			////////////////////////////////////////////////

			//	Local World : 부모 뼈대에서의 상대 위치
			bone->LocalTransform = Utility::ToMatrix(node->EvaluateLocalTransform());
			//	World(Global) : 전체 월드
			bone->GlobalTransform = Utility::ToMatrix(node->EvaluateGlobalTransform());

			boneDatas.push_back(bone);

			if (nodeType == FbxNodeAttribute::eMesh)
			{
				//	메쉬를 삼각형 단위로 바꾼다.
				converter->Triangulate
				(
					attribute,	//	현재 노드의 속성(데이터)
					true,		//	바꿔서 씬에다 넣어라 
					true		//	false로 하면 요즘 버전 방식으로 바뀜
								//	true로 하면 이전 버전 방식으로 사용
				);

				//	ReadMeshData(현재 노드, 메쉬가 소속될 bone 인덱스)
				ReadMeshData(node, index);
			}
		}//if(b)
	}

	//	자식의 갯수 만큼 재귀 호출
	for (int i = 0; i < node->GetChildCount(); i++)
		ReadBoneData(node->GetChild(i), boneDatas.size(), index);
}

void Fbx::Exporter::ReadMeshData(FbxNode * node, int parentBone)
{
	//	GetMesh() : 메쉬를 받아옴
	//	node의 attribute가 eMesh가 아니면 NULL 반환
	FbxMesh* mesh = node->GetMesh();

	vector<FbxVertex *> vertices;
	//	p : 폴리곤
	//	GetPolygonCount : 삼격형의 갯수
	for (int p = 0; p < mesh->GetPolygonCount(); p++)
	{
		//	GetPolygonSize(p) : 폴리곤의 버텍스 수
		int vertexInPolygon = mesh->GetPolygonSize(p);
		//	갯수가 3이 아니면 assert
		assert(vertexInPolygon == 3);

		//	vertex index
		//	오른손 좌표계로 되어있기 때문에 뒤집음
		for (int vi = vertexInPolygon - 1; vi >= 0; vi--)
		{
			FbxVertex* vertex = new FbxVertex();

			//	컨트롤 포인트 인덱스
			//	속성을 Triangulate함수를 통해 삼각형화 하면 변환된
			//	Fbx의 내부 데이터가 어떻게 되어있는지 모르기 때문에
			//	Control Point를 통해 데이터에 접근 해야한다.
			//	GetPolygonVertex(폴리곤 번호, 버텍스 번호)
			int cpIndex = mesh->GetPolygonVertex(p, vi);
			vertex->ControlPoint = cpIndex;

			D3DXVECTOR3 temp;

			//	GetControlPointAt(cpIndex)
			//	컨트롤 포인트 번호를 넘겨주면 해당 정점의 위치를 반환한다.
			FbxVector4 position = mesh->GetControlPointAt(cpIndex);
			//	FbxVector4 -> D3DXVECTOR3
			temp = Utility::ToVector3(position);
			//	오른손 좌표계의 정점을 왼손 좌표계의 정점으로 바꿈
			//	오른손 좌표계의 정점을 그대로 출력하면 거꾸로 나오기 때문에
			//	반대로(Utility::Negative) 뒤집는다.
			D3DXVec3TransformCoord(&vertex->Vertex.Position, &temp, &Utility::Negative());

			FbxVector4 normal;
			//	노말 값을 받아옴
			mesh->GetPolygonVertexNormal(p, vi, normal);
			//	방향은 크기가 의미가 없기 때문에
			//	0 ~ 1사이로 정규화시킨다.
			normal.Normalize();
			temp = Utility::ToVector3(normal);
			D3DXVec3TransformCoord(&vertex->Vertex.Normal, &temp, &Utility::Negative());

			//	해당 위치에 있는 정점이 소속된 머터리얼을 가져옴
			vertex->MaterialName = Utility::GetMaterialName(mesh, p, cpIndex);

			//	UV가 저장된 번호
			int uvIndex = mesh->GetTextureUVIndex(p, vi);
			vertex->Vertex.Uv = Utility::GetUv(mesh, cpIndex, uvIndex);

			vertices.push_back(vertex);
		}//	for(vi)
		UINT index0 = vertices.size() - 3;
		UINT index1 = vertices.size() - 2;
		UINT index2 = vertices.size() - 1;

		D3DXVECTOR3 p0 = vertices[index0]->Vertex.Position;
		D3DXVECTOR3 p1 = vertices[index1]->Vertex.Position;
		D3DXVECTOR3 p2 = vertices[index2]->Vertex.Position;

		D3DXVECTOR2 uv0 = vertices[index0]->Vertex.Uv;
		D3DXVECTOR2 uv1 = vertices[index1]->Vertex.Uv;
		D3DXVECTOR2 uv2 = vertices[index2]->Vertex.Uv;

		D3DXVECTOR3 e0 = p1 - p0;
		D3DXVECTOR3 e1 = p2 - p0;

		float u0 = uv1.x - uv0.x;
		float u1 = uv2.x - uv0.x;
		float v0 = uv1.y - uv0.y;
		float v1 = uv2.y - uv0.y;
		float r = 1.0f / (u0 * v1 - v0 * u1);

		D3DXVECTOR3 tangent;
		tangent.x = r * (v1 * e0.x - v0 * e1.x);
		tangent.y = r * (v1 * e0.y - v0 * e1.y);
		tangent.z = r * (v1 * e0.z - v0 * e1.z);

		vertices[index0]->Vertex.Tangent += tangent;
		vertices[index1]->Vertex.Tangent += tangent;
		vertices[index2]->Vertex.Tangent += tangent;



	}//	for(p)

	for (UINT i = 0; i < vertices.size(); i++)
	{
		D3DXVECTOR3 n = vertices[i]->Vertex.Normal;
		D3DXVECTOR3 t = vertices[i]->Vertex.Tangent;

		D3DXVECTOR3 temp = (t - n * D3DXVec3Dot(&n, &t));
		D3DXVec3Normalize(&temp, &temp);

		vertices[i]->Vertex.Tangent = temp;
	}

	FbxMeshData* meshData = new FbxMeshData();
	meshData->Name = node->GetName();
	meshData->ParentBone = parentBone;
	meshData->Vertices = vertices;
	meshData->Mesh = mesh;
	meshDatas.push_back(meshData);
}

void Fbx::Exporter::ReadSkinData()
{
	////////////////////////////////////////////////
	MyGui::log.AddLog("\nReading Skins...\n");
	MyGui::log.AddLog("===============================================\n");
	////////////////////////////////////////////////

	//	정점을 메쉬파트 별로 분할
	for (FbxMeshData* meshData : meshDatas)
	{
		FbxMesh* mesh = meshData->Mesh;

		int deformerCount = mesh->GetDeformerCount();
		this->deformerCount = deformerCount;
		//	vector<x> v(n,x) vector.assign(n, x)과 같음
		vector<FbxBoneWeights> boneWeights(mesh->GetControlPointsCount(), FbxBoneWeights());

		////////////////////////////////////////////////
		MyGui::log.AddLog("Deformer Count : %d\n", deformerCount);
		////////////////////////////////////////////////

		for (int i = 0; i < deformerCount; i++)
		{
			//	가중치에 의해 변경될 지점
			//	지점에 스킨들이 들어있다.
			//	eSkin : 스키닝만 가져옴
			FbxDeformer * deformer = mesh->GetDeformer(i, FbxDeformer::eSkin);

			FbxSkin *skin = reinterpret_cast<FbxSkin*>(deformer);
			if (skin == NULL) continue;

			////////////////////////////////////////////////
			MyGui::log.AddLog("Cluster Count : %d\n", skin->GetClusterCount());
			////////////////////////////////////////////////

			//	Cluster : 스킨이 영향을 받을 연결고리
			for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
			{
				FbxCluster* cluster = skin->GetCluster(clusterIndex);
				assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

				//	cluster의 영향을 받을 본의 이름
				string linkName = cluster->GetLink()->GetName();
				UINT boneIndex = GetBoneIndexByName(linkName);

				////////////////////////////////////////////////
				MyGui::log.AddLog("Name : %s, ", linkName.c_str());
				MyGui::log.AddLog("Bone Index : %d\n", (int)boneIndex);
				////////////////////////////////////////////////

				FbxAMatrix transform;
				FbxAMatrix linkTransform;

				cluster->GetTransformMatrix(transform);
				cluster->GetTransformLinkMatrix(linkTransform);

				//	cluster의 영향을 받는 본의 공간
				boneDatas[boneIndex]->LocalTransform = Utility::ToMatrix(transform);
				boneDatas[boneIndex]->GlobalTransform = Utility::ToMatrix(linkTransform);

				//	cluster의 영향을 받는 인덱스 번호
				////////////////////////////////////////////////
				MyGui::log.AddLog("CP Indices Count : %d\n", cluster->GetControlPointIndicesCount());
				////////////////////////////////////////////////

				for (int indexCount = 0; indexCount < cluster->GetControlPointIndicesCount(); indexCount++)
				{
					int temp = cluster->GetControlPointIndices()[indexCount];
					double* weights = cluster->GetControlPointWeights();

					//	정점이 본의 위치로 변할때 줄 가중치
					boneWeights[temp].AddWeights(boneIndex, (float)weights[indexCount]);
				}
			}// for(cluster index)
		}//	for(deformer)

		for (UINT i = 0; i < boneWeights.size(); i++)
			boneWeights[i].Normalize();

		for (FbxVertex* vertex : meshData->Vertices)
		{
			int cpIndex = vertex->ControlPoint;

			FbxBlendWeight weights;
			boneWeights[cpIndex].GetBlendWeights(weights);
			vertex->Vertex.BlendIndices = weights.Indices;
			vertex->Vertex.BlendWeights = weights.Weights;
		}


		for (int i = 0; i < scene->GetMaterialCount(); i++)
		{
			FbxSurfaceMaterial * material = scene->GetMaterial(i);
			string materialName = material->GetName();

			vector<FbxVertex *> gather;
			for (FbxVertex* temp : meshData->Vertices)
			{
				//	해당 머터리얼에 있는 버텍스 검출
				if (temp->MaterialName == materialName)
					gather.push_back(temp);
			}
			if (gather.size() < 1) continue;

			//	실제 그려줄 정보
			FbxMeshPartData* meshPart = new FbxMeshPartData();
			meshPart->MaterialName = materialName;

			for (FbxVertex* temp : gather)
			{
				ModelVertexType vertex;
				vertex = temp->Vertex;

				meshPart->Vertices.push_back(vertex);
				meshPart->Indices.push_back(meshPart->Indices.size());
			}
			meshData->MeshParts.push_back(meshPart);
		}
	}//	for(MeshData)
}

void Fbx::Exporter::WriteMesh(wstring saveFolder, wstring fileName)
{
	//	폴더 생성
	Path::CreateFolder(saveFolder);

	BinaryWriter* w = new BinaryWriter();
	//	저장할 파일 열기
	w->Open(saveFolder + fileName);

	//	본데이터가 몇개 인지
	w->UInt(boneDatas.size());
	for (FbxBoneData * bone : boneDatas)
	{
		w->Int(bone->Index);
		w->String(bone->Name);
		w->Int(bone->Parent);

		w->Matrix(bone->LocalTransform);
		w->Matrix(bone->GlobalTransform);

		SAFE_DELETE(bone);
	}

	w->UInt(meshDatas.size());
	for (FbxMeshData* meshData : meshDatas)
	{
		w->String(meshData->Name);
		w->Int(meshData->ParentBone);

		w->UInt(meshData->MeshParts.size());
		for (FbxMeshPartData* part : meshData->MeshParts)
		{
			w->String(part->MaterialName);

			w->UInt(part->Vertices.size());
			//	벡터의 자체의 주소지 말고
			//	벡터의 0번지 주소를 보내야함
			w->Byte(&part->Vertices[0], sizeof(ModelVertexType) * part->Vertices.size());

			w->UInt(part->Indices.size());
			w->Byte(&part->Indices[0], sizeof(UINT) * part->Indices.size());

			SAFE_DELETE(part);
		}

		SAFE_DELETE(meshData);
	}

	w->Close();
	SAFE_DELETE(w);
}


Fbx::FbxClip* Fbx::Exporter::ReadAnimationData(UINT index)
{
	int stackCount = importer->GetAnimStackCount();
	assert(index < (UINT)stackCount);

	////////////////////////////////////////////////
	MyGui::log.AddLog("Clip Count : %d\n", stackCount);
	////////////////////////////////////////////////

	//	GetGlobalSettings : 씬에 세팅된 모든 정보를 가지고 있음
	//	GetTimeMode : 현제 씬에 사용중인 Time Mode를 반환함
	FbxTime::EMode mode = scene->GetGlobalSettings().GetTimeMode();
	//	현재 Time Mode의 frameRate를 받아옴
	float frameRate = (float)FbxTime::GetFrameRate(mode);

	////////////////////////////////////////////////
	MyGui::log.AddLog("FrameRate : %.2f\n", frameRate);
	////////////////////////////////////////////////

	FbxClip *clip = new FbxClip();
	clip->FrameRate = frameRate;

	//	Take : 애니메이션 동작의 묶음(이름으로)
	//	GetTakeInfo : index에 맞는 Take 정보를 반환함
	FbxTakeInfo *takeInfo = importer->GetTakeInfo(index);
	clip->Name = takeInfo->mName.Buffer();

	////////////////////////////////////////////////
	MyGui::log.AddLog("Clip Name : %s\n", clip->Name.c_str());
	////////////////////////////////////////////////

	//	Time Span : 애니메이션읜 시간 간격
	FbxTimeSpan span = takeInfo->mLocalTimeSpan;
	//	Duration : Time Span 의 절대값
	FbxTime tempDuration = span.GetDuration();

	//	GetStart : TimeSpan의 시작 시간
	//	GetStop : TimeSpan의 끝 시간
	//	GetFrameCount : 해당 시간의 프레임
	//	GetSecondDouble : 시간을 초로 반환
	int start = (int)span.GetStart().GetFrameCount();
	int stop = (int)span.GetStop().GetFrameCount();
	float duration = (float)tempDuration.GetMilliSeconds();

	////////////////////////////////////////////////
	MyGui::log.AddLog("Start[%d], Stop[%d], Duration[%.2f]\n", start, stop, duration);
	////////////////////////////////////////////////

	if (start < stop)
		ReadKeyframeData(clip, scene->GetRootNode(), start, stop, -1, -1);

	clip->Duration = duration;
	clip->FrameCount = (stop - start) + 1;

	////////////////////////////////////////////////
	MyGui::log.AddLog("Frame Count : %d\n", clip->FrameCount);
	////////////////////////////////////////////////

	return clip;
}

Fbx::FbxClip* Fbx::Exporter::ReadAnimationData(wstring name)
{
	UINT number = -1;
	//	GetAnimStackCount : 애니메이션의 갯수
	for (int i = 0; i < importer->GetAnimStackCount(); i++)
	{
		//	Take : 애니메이션 동작의 묶음(이름으로)
		//	GetTakeInfo : index에 맞는 Take 정보를 반환함
		FbxTakeInfo *takeInfo = importer->GetTakeInfo(i);
		wstring compare = String::ToWString(takeInfo->mName.Buffer());

		if (name == compare)
		{
			number = i;

			break;
		}
	}

	if (number != -1)
		return ReadAnimationData(number);

	return NULL;
}

void Fbx::Exporter::ReadKeyframeData(FbxClip * clip, FbxNode * node, int start, int stop, int index, int parent)
{
	//	FbxNodeAttribute : 현재 노드가 어떤 데이터를 가지고 있는지
	FbxNodeAttribute * attribute = node->GetNodeAttribute();

	if (attribute != NULL)
	{
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();
		if (nodeType == FbxNodeAttribute::eSkeleton)
		{
			FbxKeyframe *keyframe = new FbxKeyframe;
			keyframe->BoneName = node->GetName();
			keyframe->Index = index;
			keyframe->ParentIndex = parent;

			for (int i = start; i <= stop; i++)
			{
				//	SetFrame : 시간을 Frame형식으로 세팅해줌
				FbxTime animationTime;
				animationTime.SetFrame(i);

				//	EvaluateLocalTransform : 시간에 맞는 Transform을 반환
				//	Affine Transformation
				//	- 변환전 물제의 속성중 평행한 선은 Affine변환
				//	- 후에도 평행하다는 것이 특징이다.
				//	- 선형 변환에 이동까지 포함
				//	- 점과 벡터에 대한 연산을 둘다할 수 있음
				//	Linear Transformation : 자유도 3(x, y, z)
				//	- 벡터 공간에서의 이동(이동 변환은 제외)
				FbxAMatrix matrix = node->EvaluateLocalTransform(animationTime);
				D3DXMATRIX transform = Utility::ToMatrix(matrix);

				FbxKeyframeData data;
				//	해당 프레임의 시간
				data.Time = (float)animationTime.GetMilliSeconds();
				D3DXMatrixDecompose(&data.Scale, &data.Rotation, &data.Translation, &transform);

				keyframe->Transforms.push_back(data);
			}

			clip->Keyframes.push_back(keyframe);
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
		ReadKeyframeData(clip, node->GetChild(i), start, stop, clip->Keyframes.size(), index);
}

void Fbx::Exporter::WriteClipData(FbxClip * clip, wstring saveFolder, wstring fileName)
{
	BinaryWriter* w = new BinaryWriter();
	w->Open(saveFolder + fileName);

	w->String(clip->Name);
	w->Float(clip->Duration);
	w->Float(clip->FrameRate);
	w->UInt(clip->FrameCount);


	w->UInt(clip->Keyframes.size());
	for (FbxKeyframe* keyframe : clip->Keyframes)
	{
		w->String(keyframe->BoneName);

		w->UInt(keyframe->Index);
		w->UInt(keyframe->ParentIndex);

		w->UInt(keyframe->Transforms.size());
		w->Byte(&keyframe->Transforms[0], sizeof(FbxKeyframeData) * keyframe->Transforms.size());

		SAFE_DELETE(keyframe);
	}

	w->Close();
	SAFE_DELETE(w);
}
