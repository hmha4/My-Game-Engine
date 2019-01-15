#include "stdafx.h"
#include "FbxLoader.h"
#include "FbxType.h"
#include "FbxUtility.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

FbxLoader::FbxLoader(wstring file, wstring savefolder, wstring saveName)
	: fbxFile(file), saveFolder(savefolder), saveName(saveName)
{
	//   매니져 생성
	manager = FbxManager::Create();
	//   씬 생성(트리 형태로 파일의 노드를 가지고 있음)
	scene = FbxScene::Create(manager, "");

	ios = FbxIOSettings::Create(manager, IOSROOT);
	//   FBX 내의 옵션 설정
	//   FBX 내의 원하는 설정의 파일을 압출 풀어 파일형태로 가져옴
	ios->SetBoolProp(IMP_FBX_TEXTURE, true);
	//   매니져에 ios 연결
	manager->SetIOSettings(ios);

	//   임포터 생성
	//   임포터에 매니져 연결
	//   "" : 이름
	importer = FbxImporter::Create(manager, "");

	string sFile = String::ToString(file);
	//   (파일 이름, 포맷번호, IOS)
	//   파일 번호 : -1 - 모든 포맷
	//            0 - FBX
	//            1 - OBJ
	//   ...

	//   importer에 대한 초기화
	//   import시킬 파일, 파일 포맷, IOS(없다면, 자동을 디폴트 ios를 만듬)
	bool b = importer->Initialize(sFile.c_str(), -1, ios);
	assert(b == true);

	//   연 파일을 씬에다가 넣음
	//   씬 밑에 루트 노드가 생김
	b = importer->Import(scene);
	assert(b == true);

	//	축
	FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
	FbxUtility::bRightHand = axis.GetCoorSystem() == FbxAxisSystem::eRightHanded;

	//	단위
	FbxSystemUnit unit = scene->GetGlobalSettings().GetSystemUnit();
	if (unit != FbxSystemUnit::m)
	{
		FbxSystemUnit::ConversionOptions option = 
		{
			false,	//	부모 노드로 부터 단위를 상속 받을 꺼냐
			false,	//	크기를 변환 할꺼냐
			false,	//	클러스터를 변환 할꺼냐
			true,	//	노말 값을 변환 할꺼냐
			true,	//	탄젠트를 변환 할꺼냐
			true,	//	카메라를 변환 할꺼냐
		};

		FbxSystemUnit::m.ConvertScene(scene, option);
	}

	//	삼각형 형태로 변환 해라
	converter = new FbxGeometryConverter(manager);
}

FbxLoader::~FbxLoader()
{
	SAFE_DELETE(converter);

	importer->Destroy();
	ios->Destroy();
	scene->Destroy();
	manager->Destroy();
}

void FbxLoader::ExportMaterial(wstring saveFolder, wstring fileName)
{
	ReadMaterial();

	//	Write
	wstring tempFolder = saveFolder.length() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.length() < 1 ? this->saveName : fileName;

	WriteMaterial(tempFolder, tempName + L".material");
}

void FbxLoader::ExportMesh(wstring saveFolder, wstring fileName)
{
	ReadBoneData(scene->GetRootNode(), -1, -1);
	ReadSkinData();

	//	Write
	wstring tempFolder = saveFolder.length() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.length() < 1 ? this->saveName : fileName;

	WriteMesh(tempFolder, tempName + L".mesh");
}


void FbxLoader::GetClipList(vector<wstring>* list)
{
	list->clear();
	for (int i = 0; i < importer->GetAnimStackCount(); i++)
	{
		FbxTakeInfo *takeInfo = importer->GetTakeInfo(i);

		string name = takeInfo->mName.Buffer();
		list->push_back(String::ToWString(name));
	}
}

void FbxLoader::ExportAnimation(UINT clipNumber, wstring saveFolder, wstring fileName)
{
	FbxClip * clip = ReadAnimationData(clipNumber);

	//	Write
	wstring tempFolder = saveFolder.length() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = fileName.length() < 1 ? this->saveName : fileName;

	WriteClipData(clip, tempFolder, tempName + L".anim");

}

void FbxLoader::ExportAnimation(wstring clipName, wstring saveFolder, wstring fileName)
{
	FbxClip * clip = ReadAnimationData(clipName);

	//	Write
	wstring tempFolder = saveFolder.length() < 1 ? this->saveFolder : saveFolder;
	wstring tempName = saveName.length() < 1 ? this->saveName : fileName;

	WriteClipData(clip, tempFolder, tempName + L".anim");
}


void FbxLoader::ReadMaterial()
{
	//   씬에서 머티리얼의 갯수를 받아옴
	//   한 Fbx에 머터리얼의 갯수는 한개 이상이다.
	int count = scene->GetMaterialCount();

	for (int i = 0; i < count; i++)
	{
		//   씬에서 각각의 머터리얼 정보를 받아옴
		FbxSurfaceMaterial * fbxMaterial = scene->GetMaterial(i);
		//   XML 형식으로 저장하기 위해
		//   구조체에 저장
		FbxMaterial * material = new FbxMaterial;
		material->Name = fbxMaterial->GetName();

		//   FbxSurfaceMaterial이 FbxSurfaceLambert로 다운캐스팅이 가능한지
		if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) == true)
		{
			// 가능하다면 다운캐스팅후 lambert정보를 받아옴
			FbxSurfaceLambert * lambert = (FbxSurfaceLambert*)fbxMaterial;

			//   lambert->Diffuse : Diffuse칼라 값(rgb)
			//   lambert->DiffuseFactor : 강도(a)
			material->Diffuse = FbxUtility::ToColor(lambert->Diffuse, lambert->DiffuseFactor);
		}
		if (fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId) == true)
		{
			FbxSurfacePhong * phong = (FbxSurfacePhong*)fbxMaterial;

			material->Specular = FbxUtility::ToColor(phong->Specular, phong->SpecularFactor);
			material->SpecularExp = phong->Shininess;
		}

		//   습성 정보
		FbxProperty prop;
		//   현재 Diffuse의 이름으로 습성을 찾아서 받아옴
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		//   Diffuse의 파일경로를 얻어옴
		material->DiffuseFile = FbxUtility::GetTextureFile(prop);

		//	현재 Specular의 이름으로 습성 찾아서 받아옴
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		material->SpecularFile = FbxUtility::GetTextureFile(prop);

		//	현재 Specular의 이름으로 습성 찾아서 받아옴
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
		material->NormalMapFile = FbxUtility::GetTextureFile(prop);

		
		materials.push_back(material);
	}
}

void FbxLoader::WriteMaterial(wstring saveFolder, wstring fileName)
{
	//   지정한 위치에 폴더를 만듬
	Path::CreateFolder(saveFolder);

	//   모든 노드는 XMLDocument에 연결된다
	//   document가 사라지면 연결된 모든 노드도 같이 사라진다.
	Xml::XMLDocument * document = new Xml::XMLDocument();
	//   Declaration(선언)이 첫 진입점이 된다.
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	//   Materials란 노드를 root 노드로 아래에
	//   Material종류 별로 연결
	Xml::XMLElement * root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (FbxMaterial* material : materials)
	{
		//   root 아래에 연결
		Xml::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);


		Xml::XMLElement* element = NULL;

		element = document->NewElement("Name");
		element->SetText(material->Name.c_str());
		node->LinkEndChild(element);


		element = document->NewElement("DiffuseFile");
		CopyTextureFile(material->DiffuseFile, saveFolder);
		element->SetText(material->DiffuseFile.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		CopyTextureFile(material->SpecularFile, saveFolder);
		element->SetText(material->SpecularFile.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		CopyTextureFile(material->NormalMapFile, saveFolder);
		element->SetText(material->NormalMapFile.c_str());
		node->LinkEndChild(element);


		element = document->NewElement("Diffuse");
		node->LinkEndChild(element);

		WriteXmlColor(element, document, material->Diffuse);

		element = document->NewElement("Specular");
		node->LinkEndChild(element);

		WriteXmlColor(element, document, material->Specular);

		element = document->NewElement("SpecularExp");
		element->SetText(material->SpecularExp);
		node->LinkEndChild(element);

		SAFE_DELETE(material);
	}

	//   파일을 지정 경로에 저장
	string file = String::ToString(saveFolder + fileName);
	document->SaveFile(file.c_str());
}

void FbxLoader::WriteXmlColor(Xml::XMLElement * element, Xml::XMLDocument * document, D3DXCOLOR color)
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

	newColor = document->NewElement("A");
	newColor->SetText(color.a);
	element->LinkEndChild(newColor);
}

void FbxLoader::CopyTextureFile(OUT string & textureFile, wstring & saveFolder)
{
	if (textureFile.length() < 1)
		return;

	wstring file = String::ToWString(textureFile);
	wstring fileName = Path::GetFileName(file);

	//   파일이 존재한다면, 경로에 한글 안됨
	if (Path::ExistFile(textureFile) == true)
		//   파일 복사 
		CopyFile(file.c_str(), (saveFolder + fileName).c_str(), FALSE);

	textureFile = String::ToString(fileName);
}

//   처음 호출 시에는 root노드가 들어감
void FbxLoader::ReadBoneData(FbxNode * node, int index, int parent)
{
	//   FbxNodeAttribute : 현재 노드가 어떤 데이터를 가지고 있는지 결정 해준다.
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	// 데이터가 있다면
	if (attribute != NULL)
	{
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();

		//   밑의 조건이 하나라도 만족하면 b 가 true가 된다.
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
			//   Local World : 부모 뼈대에서의 상대 위치
			bone->LocalTransform = FbxUtility::ToMatrix(node->EvaluateLocalTransform());
			//   World(Global) : 전체 월드
			bone->GlobalTransform = FbxUtility::ToMatrix(node->EvaluateGlobalTransform());

			boneDatas.push_back(bone);

			if (nodeType == FbxNodeAttribute::eMesh)
			{
				converter->Triangulate(attribute, true, true);

				ReadMeshData(node, index);
			}
		}//if(b)
	}

	//   자식의 갯수 만큼 재귀 호출
	for (int i = 0; i < node->GetChildCount(); i++)
		ReadBoneData(node->GetChild(i), boneDatas.size(), index);
}

UINT FbxLoader::GetBoneIndexByName(string name)
{
	for (UINT i = 0; i < boneDatas.size(); i++)
	{
		if (boneDatas[i]->Name == name)
			return i;
	}

	return 0;
}

void FbxLoader::ReadMeshData(FbxNode * node, int parentBone)
{
	//   GetMesh() : 메쉬를 받아옴
	//   node의 attribute가 eMesh가 아니면 NULL 반환
	FbxMesh* mesh = node->GetMesh();

	//   스킨 애니메이션아니면 0이 반환됨
	int deformerCount = mesh->GetDeformerCount();
	for (int i = 0; i < deformerCount; i++)
	{
		FbxDeformer* deformer = mesh->GetDeformer(i, FbxDeformer::eSkin);

		//   TODO : cast들(면접)
		FbxSkin* skin = reinterpret_cast<FbxSkin *>(deformer);
		if (skin == NULL) continue;

		//   Cluster : 연결고리
		for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);
			assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

			//   cluster의 영향을 받을 본의 이름
			string linkName = cluster->GetLink()->GetName();
			UINT boneIndex = GetBoneIndexByName(linkName);

			FbxAMatrix transform;
			FbxAMatrix linkTransform;

			cluster->GetTransformMatrix(transform);
			cluster->GetTransformLinkMatrix(linkTransform);

			//   cluster의 영향을 받는 본의 공간
			boneDatas[boneIndex]->LocalTransform = FbxUtility::ToMatrix(transform);
			boneDatas[boneIndex]->GlobalTransform = FbxUtility::ToMatrix
			(
				linkTransform.Inverse() * transform
			);

			int * indices = cluster->GetControlPointIndices();
			double * weights = cluster->GetControlPointWeights();

			//   cluster의 영향을 받는 인덱스 번호
			//   한점에서 반복적으로 나올수도 있다.
			/*for (int indexCount = 0; indexCount < cluster->GetControlPointIndicesCount(); indexCount++)
			{
				FbxVertexWeightData vertexWeight;
				vertexWeight.Index = boneIndex;
				vertexWeight.Weight = weights[indexCount];
				vertexWeight.Index = indices[indexCount];

				UINT cpIndex = indices[indexCount];
				cpDatas[cpIndex].Datas.push_back(vertexWeight);
			}*/
		}// for(cluster index)
	}//   for(deformer)


	vector<FbxVertex *> vertices;
	//   p : 폴리곤
	//   GetPolygonCount : 삼격형의 갯수
	for (int p = 0; p < mesh->GetPolygonCount(); p++)
	{
		//   GetPolygonSize(p) : 폴리곤의 버텍스 수
		int vertexInPolygon = mesh->GetPolygonSize(p);
		//   갯수가 3이 아니면 assert
		assert(vertexInPolygon == 3);

		//   vertex index
		//   오른손 좌표계로 되어있기 때문에 뒤집음
		for (int vi = 0; vi < 3; vi++)
		{
			int pvi = FbxUtility::bRightHand ? 2 - vi : vi;
			int cpIndex = mesh->GetPolygonVertex(p, pvi);

			FbxVertex* vertex = new FbxVertex();

			//   컨트롤 포인트 인덱스
			//   속성을 Triangulate함수를 통해 삼각형화 하면 변환된
			//   Fbx의 내부 데이터가 어떻게 되어있는지 모르기 때문에
			//   Control Point를 통해 데이터에 접근 해야한다.
			//   GetPolygonVertex(폴리곤 번호, 버텍스 번호)
			//int cpIndex = mesh->GetPolygonVertex(p, vi);
			vertex->ControlPoint = cpIndex;

			FbxVector4 position = mesh->GetControlPointAt(cpIndex);
			//D3DXVECTOR3 temp;

			//   GetControlPointAt(cpIndex)
			//   컨트롤 포인트 번호를 넘겨주면 해당 정점의 위치를 반환한다.
			//FbxVector4 position = mesh->GetControlPointAt(cpIndex);
			vertex->Vertex.Position = FbxUtility::ToPosition(position);
			//   FbxVector4 -> D3DXVECTOR3
			//temp = FbxUtility::ToVector3(position);
			//   오른손 좌표계의 정점을 왼손 좌표계의 정점으로 바꿈
			//   오른손 좌표계의 정점을 그대로 출력하면 거꾸로 나오기 때문에
			//   반대로(FbxUtility::Negative) 뒤집는다.
			//D3DXVec3TransformCoord(&vertex->Vertex.Position, &temp, &FbxUtility::Negative());

			FbxVector4 normal;
			//   노말 값을 받아옴
			mesh->GetPolygonVertexNormal(p, pvi, normal);
			//   방향은 크기가 의미가 없기 때문에
			//   0 ~ 1사이로 정규화시킨다.
			normal.Normalize();
			vertex->Vertex.Normal = FbxUtility::ToNormal(normal);
			//temp = FbxUtility::ToVector3(normal);
			//D3DXVec3TransformCoord(&vertex->Vertex.Normal, &temp, &FbxUtility::Negative());

			//   해당 위치에 있는 정점이 소속된 머터리얼을 가져옴
			vertex->MaterialName = FbxUtility::GetMaterialName(mesh, p, cpIndex);

			//   UV가 저장된 번호
			int uvIndex = mesh->GetTextureUVIndex(p, pvi);
			vertex->Vertex.Uv = FbxUtility::GetUv(mesh, cpIndex, uvIndex);

			vertices.push_back(vertex);
		}//   for(vi)

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

	}//   for(p)

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

void FbxLoader::ReadSkinData()
{
	for (FbxMeshData* meshData : meshDatas)
	{
		FbxMesh* mesh = meshData->Mesh;


		int deformerCount = mesh->GetDeformerCount();

		vector<FbxBoneWeights> boneWeights;
		boneWeights.assign(mesh->GetControlPointsCount(), FbxBoneWeights());

		for (int i = 0; i < deformerCount; i++)
		{
			//	deformer : cluster 관리
			FbxDeformer* deformer = mesh->GetDeformer(i, FbxDeformer::eSkin);

			FbxSkin* skin = reinterpret_cast<FbxSkin *>(deformer);
			if (skin == NULL) continue;

			for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
			{
				//	cluster : 관절
				FbxCluster* cluster = skin->GetCluster(clusterIndex);
				assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

				//	GetLink : 관절에 연결된 본
				string linkName = cluster->GetLink()->GetName();
				UINT boneIndex = GetBoneIndexByName(linkName);


				FbxAMatrix transform;
				FbxAMatrix linkTransform;

				cluster->GetTransformMatrix(transform);
				cluster->GetTransformLinkMatrix(linkTransform);

				//	Transform : 본이 아닌 관절의 Transform
				boneDatas[boneIndex]->LocalTransform = FbxUtility::ToMatrix(transform);
				boneDatas[boneIndex]->GlobalTransform = FbxUtility::ToMatrix(linkTransform);


				for (int indexCount = 0; indexCount < cluster->GetControlPointIndicesCount(); indexCount++)
				{
					int temp = cluster->GetControlPointIndices()[indexCount];
					//	본에 있는 컨트롤 포인트가 아닌 관절에 있는 컨트롤 포인트의 Weight
					double* weights = cluster->GetControlPointWeights();

					boneWeights[temp].AddWeights(boneIndex, (float)weights[indexCount]);
				}
			}//for(clusterIndex)
		}//for(deformer)

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
			FbxSurfaceMaterial* material = scene->GetMaterial(i);
			string materialName = material->GetName();

			vector<FbxVertex *> gather;
			for (FbxVertex* temp : meshData->Vertices)
			{
				if (temp->MaterialName == materialName)
					gather.push_back(temp);
			}
			if (gather.size() < 1) continue;


			FbxMeshPartData* meshPart = new FbxMeshPartData();
			meshPart->MaterialName = materialName;

			for (FbxVertex* temp : gather)
			{
				VertexTextureNormalTangentBlend vertex;
				vertex = temp->Vertex;

				meshPart->Vertices.push_back(vertex);
				meshPart->Indices.push_back(meshPart->Indices.size());
			}
			meshData->MeshParts.push_back(meshPart);
		}
	}//for(MeshData)
}

void FbxLoader::WriteMesh(wstring saveFolder, wstring fileName)
{
	//   폴더 생성
	Path::CreateFolder(saveFolder);

	BinaryWriter* w = new BinaryWriter();
	//   저장할 파일 열기
	w->Open(saveFolder + fileName);

	//   본데이터가 몇개 인지
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
			//   벡터의 자체의 주소지 말고
			//   벡터의 0번지 주소를 보내야함
			w->Byte(&part->Vertices[0], sizeof(VertexTextureNormalTangentBlend) * part->Vertices.size());

			w->UInt(part->Indices.size());
			w->Byte(&part->Indices[0], sizeof(UINT) * part->Indices.size());

			SAFE_DELETE(part);
		}

		SAFE_DELETE(meshData);
	}

	w->Close();
	SAFE_DELETE(w);
}

FbxClip* FbxLoader::ReadAnimationData(wstring name)
{
	UINT number = -1;
	//   GetAnimStackCount : 애니메이션 갯수
	for (int i = 0; i < importer->GetAnimStackCount(); i++)
	{
		//   GetTakeInfo : 동작
		// 해당 동작의 이름을 받아서 받아온 이름과 비교후 
		// 맞으면 number에 인덱스전달
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

FbxClip* FbxLoader::ReadAnimationData(UINT index)
{
	UINT stackCount = (UINT)importer->GetAnimStackCount();
	assert(index < stackCount);

	//   GetGlobalSettings : 씬에 세팅 되어있는 모든 정보를 가져옴
	FbxTime::EMode mode = scene->GetGlobalSettings().GetTimeMode();
	// 속도를 받아옴
	float frameRate = (float)FbxTime::GetFrameRate(mode);

	//에니메이션 클립데이터 생성
	FbxClip *clip = new FbxClip();
	clip->FrameRate = frameRate;

	//   GetTakeInfo : 동작
	// 번호에 해당하는 동작
	FbxTakeInfo *takeInfo = importer->GetTakeInfo(index);
	clip->Name = takeInfo->mName.Buffer();
	
	//   현재 애니메이션의 시간 간격
	FbxTimeSpan span = takeInfo->mLocalTimeSpan;
	FbxTime tempDuration = span.GetDuration();

	int start = (int)span.GetStart().GetFrameCount();		//에니메이션 시작시간
	int stop = (int)span.GetStop().GetFrameCount();			//에니메이션 종료시간
	float duration = (float)tempDuration.GetMilliSeconds();	//에니메이션 지속시간

	if (start < stop)
		ReadKeyframeData(clip, scene->GetRootNode(), start, stop, -1, -1);

	clip->Duration = duration;
	clip->FrameCount = (stop - start) + 1;	//끝에서 시작뺐으니 1더해주는것

	return clip;
}

//모든 본의 키프레임 데이터를 읽는다
void FbxLoader::ReadKeyframeData(struct FbxClip* clip, FbxNode* node, int start, int stop, int index, int parent)
{
	//FbxMesh * currMesh = node->GetMesh();
	//UINT deformerCount = currMesh->GetDeformerCount();
	//
	//FbxAMatrix geometryTransform = FbxUtility::GetGeometryTransformation(node);
	//
	//for (UINT deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++)
	//{
	//	FbxDeformer* deformer = currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin);
	//	FbxSkin* skin = reinterpret_cast<FbxSkin *>(deformer);
	//	if (!skin) continue;
	//
	//	UINT clusterCount = skin->GetClusterCount();
	//
	//	for (UINT clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
	//	{
	//		FbxCluster * cluster = skin->GetCluster(clusterIndex);

			//노드의 정보를 가져온다
			FbxNodeAttribute* attribute = node->GetNodeAttribute();
			if (attribute != NULL)
			{
				FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();
				//노드타입이 스켈레톤일때
				if (nodeType == FbxNodeAttribute::eSkeleton)
				{
					//키프레임 생성
					FbxKeyframe* keyframe = new FbxKeyframe();
					keyframe->BoneName = node->GetName();
					keyframe->index = index;
					keyframe->parent = parent;


					//시간당 한프레임씩 키프레임 데이터를 저장한다
					for (int i = start; i <= stop; i++)
					{
						//   SetFrame : 0~frame의 시간 계산
						FbxTime animationTime;
						animationTime.SetFrame(i);
						//   TODO : affine 변환


						//FbxAMatrix matrix = node->EvaluateGlobalTransform(animationTime) * geometryTransform;
						//matrix = matrix.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(animationTime);

						FbxAMatrix matrix =  node->EvaluateLocalTransform(animationTime);
						D3DXMATRIX transform = FbxUtility::ToMatrix(matrix);

						FbxKeyframeData data;
						data.Time = (float)animationTime.GetMilliSeconds();
						D3DXMatrixDecompose(&data.Scale, &data.Rotation, &data.Translation, &transform);

						keyframe->Transforms.push_back(data);
					}
					//클립에 키프레임 삽입
					clip->KeyFrames.push_back(keyframe);
				}//if(nodeType)
			}
			//자식의 갯수만큼 반복 => 허리->다리->발 이런식으로
			for (int i = 0; i < node->GetChildCount(); i++)
				ReadKeyframeData(clip, node->GetChild(i), start, stop, clip->KeyFrames.size(), index);
	//	}
	//	
	//}
}

void FbxLoader::WriteClipData(FbxClip * clip, wstring saveFolder, wstring fileName)
{
	BinaryWriter* w = new BinaryWriter();
	w->Open(saveFolder + fileName);

	w->String(clip->Name);
	w->Float(clip->Duration);
	w->Float(clip->FrameRate);
	w->UInt(clip->FrameCount);

	w->UInt(clip->KeyFrames.size());
	for (FbxKeyframe* keyframe : clip->KeyFrames)
	{
		w->String(keyframe->BoneName);
		w->UInt(keyframe->index);
		w->UInt(keyframe->parent);

		w->UInt(keyframe->Transforms.size());
		w->Byte(&keyframe->Transforms[0], sizeof(FbxKeyframeData) * keyframe->Transforms.size());

		SAFE_DELETE(keyframe);
	}

	w->Close();
	SAFE_DELETE(w);
}