#include "stdafx.h"
#include "FbxLoader.h"
#include "FbxType.h"
#include "FbxUtility.h"
#include "Utilities/Xml.h"
#include "Utilities/BinaryFile.h"

FbxLoader::FbxLoader(wstring file, wstring savefolder, wstring saveName)
	: fbxFile(file), saveFolder(savefolder), saveName(saveName)
{
	//	Create manager
	manager = FbxManager::Create();
	//	Create scene(It contains nodes including root node)
	scene = FbxScene::Create(manager, "");
	//	Create input output settings
	//	It is a collection of properties, arranged as a tree, 
	//	that can be used by FBX file readers and writers to represent 
	//	import and export options
	ios = FbxIOSettings::Create(manager, IOSROOT);
	//	Set boolean import "Texture" option true
	ios->SetBoolProp(IMP_FBX_TEXTURE, true);
	//	Set IOSettings pointer
	manager->SetIOSettings(ios);

	//	Create importer
	//	Imports FBX files into SDK objects
	//	"" : name
	importer = FbxImporter::Create(manager, "");

	string sFile = String::ToString(file);

	//	importer->Initialize(file name, format number, ios)
	//	format number	-	-1 : all format
	//					-	 0 : FBX format
	//					-	 1 : OBJ format
	//	if there's no ios creates default ios
	bool b = importer->Initialize(sFile.c_str(), -1, ios);
	assert(b == true);

	//	Imports currently opened file into a scene
	b = importer->Import(scene);
	assert(b == true);

	//	Coordinate system of the scene
	//	Can be converted into other coordinate systems
	FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
	FbxUtility::bRightHand = axis.GetCoorSystem() == FbxAxisSystem::eRightHanded;

	//	Units of measurement used within particular scene
	FbxSystemUnit unit = scene->GetGlobalSettings().GetSystemUnit();
	if (unit != FbxSystemUnit::m)
	{
		FbxSystemUnit::ConversionOptions option =
		{
			false,	//	Will it inherit unit from the parent node
			false,	//	Will it change the size info
			false,	//	Will it change the cluster info
			true,	//	Will it change the normal values
			true,	//	Will it change the tangent values
			true,	//	Will it change the camera clip planes
		};

		FbxSystemUnit::m.ConvertScene(scene, option);
	}

	//	Convert into Triangular form
	converter = new FbxGeometryConverter(manager);
}

FbxLoader::~FbxLoader()
{
	materials.clear();
	materials.shrink_to_fit();
	boneDatas.clear();
	boneDatas.shrink_to_fit();
	meshDatas.clear();
	meshDatas.shrink_to_fit();

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
	//	Get the number of materials from the scene
	//	There are over one materials in one FBX
	int count = scene->GetMaterialCount();

	for (int i = 0; i < count; i++)
	{
		//	Get material datas(settings) from the scene
		FbxSurfaceMaterial * fbxMaterial = scene->GetMaterial(i);

		FbxMaterial * material = new FbxMaterial;
		material->Name = fbxMaterial->GetName();

		//	Check if it's possible to down cast 
		//	from FbxSurfaceMaterial to FbxSurfaceLambert
		if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) == true)
		{
			//	Get Lambert information
			FbxSurfaceLambert * lambert = reinterpret_cast<FbxSurfaceLambert *>(fbxMaterial);

			//	Lambert->Diffuse : diffuse color (RGB)
			//	Lambert->DiffuseFactor : intensity (A)
			material->Diffuse = FbxUtility::ToColor(lambert->Diffuse, lambert->DiffuseFactor);
		}
		if (fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId) == true)
		{
			//	Get Phong information
			FbxSurfacePhong * phong = (FbxSurfacePhong*)fbxMaterial;

			material->Specular = FbxUtility::ToColor(phong->Specular, phong->SpecularFactor);
			material->SpecularExp = (float)phong->Shininess;
		}

		//	Property information
		FbxProperty prop;
		//	Get property with the current diffuse file name
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		//	Get directory of the diffuse file
		material->DiffuseFile = FbxUtility::GetTextureFile(prop);

		//	Get property with the current specular file name
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		material->SpecularFile = FbxUtility::GetTextureFile(prop);

		//	Get property with the current normal file name
		prop = fbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
		material->NormalMapFile = FbxUtility::GetTextureFile(prop);

		materials.push_back(material);
	}
}

void FbxLoader::WriteMaterial(wstring saveFolder, wstring fileName)
{
	//	Creates a folder in a specified directory
	Path::CreateFolder(saveFolder);

	//	All nodes are connected to XMLDocument
	//	If the document is delete all nodes are deleted
	Xml::XMLDocument * document = new Xml::XMLDocument();
	//	Declaration is the first entry point
	Xml::XMLDeclaration * decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	Xml::XMLElement * root = document->NewElement("Materials");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	document->LinkEndChild(root);

	for (FbxMaterial* material : materials)
	{
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

	//	Save file to a specified directory
	string file = String::ToString(saveFolder + fileName);
	document->SaveFile(file.c_str());

	SAFE_DELETE(document);
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

	//	Copy file if the file exist
	if (Path::ExistFile(textureFile) == true)
		CopyFile(file.c_str(), (saveFolder + fileName).c_str(), FALSE);

	textureFile = String::ToString(fileName);
}

//	Root node is the first entry
void FbxLoader::ReadBoneData(FbxNode * node, int index, int parent)
{
	//	FbxNodeAttribute : Data of the current node
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	//	If the node contains an attribute
	if (attribute != NULL)
	{
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();

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
			//   Local World : Relative Transforms
			bone->LocalTransform = FbxUtility::ToMatrix(node->EvaluateLocalTransform());
			//   World(Global) : Absolute Transforms
			bone->GlobalTransform = FbxUtility::ToMatrix(node->EvaluateGlobalTransform());

			boneDatas.push_back(bone);

			if (nodeType == FbxNodeAttribute::eMesh)
			{
				converter->Triangulate(attribute, true, true);

				ReadMeshData(node, index);
			}
		}//	if(b)
	}

	//	Recursively call this function
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
	//	Returns NULL if the node's attribute is not eMesh
	FbxMesh* mesh = node->GetMesh();

	vector<FbxVertex *> vertices;
	//	p : polygon
	//	GetPolygonCount : the number of the polygons
	for (int p = 0; p < mesh->GetPolygonCount(); p++)
	{
		//	GetPolygonSize(p) : the number of the vertices in a polygon
		int vertexInPolygon = mesh->GetPolygonSize(p);
		//	The number of the vertices has to be 3
		assert(vertexInPolygon == 3);

		for (int vi = 0; vi < 3; vi++)
		{
			//	If it's create in a right hand coordinate system
			//! Go opposite!!
			int pvi = FbxUtility::bRightHand ? 2 - vi : vi;
			//	cpIndex : Control point index
			//	It needs to be accessed to position, normal, material, UV
			//	with the control point if the attribute is triangulated 
			//	because we don't know how the data is created
			int cpIndex = mesh->GetPolygonVertex(p, pvi);

			FbxVertex* vertex = new FbxVertex();
			vertex->ControlPoint = cpIndex;

			//	Get corresponding position of the control point
			FbxVector4 position = mesh->GetControlPointAt(cpIndex);
			vertex->Vertex.Position = FbxUtility::ToPosition(position);
			
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(p, pvi, normal);
			normal.Normalize();
			vertex->Vertex.Normal = FbxUtility::ToNormal(normal);

			//	Get corresponding material of the control point
			vertex->MaterialName = FbxUtility::GetMaterialName(mesh, p, cpIndex);

			//	Get corresponding UV datas of the control point
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
			//	deformer : cluster manager
			FbxDeformer* deformer = mesh->GetDeformer(i, FbxDeformer::eSkin);

			FbxSkin* skin = reinterpret_cast<FbxSkin *>(deformer);
			if (skin == NULL) continue;

			for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++)
			{
				//	cluster : joint
				FbxCluster* cluster = skin->GetCluster(clusterIndex);
				assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

				//	GetLink : Get the bone linked to the cluster
				string linkName = cluster->GetLink()->GetName();
				UINT boneIndex = GetBoneIndexByName(linkName);

				FbxAMatrix transform;
				FbxAMatrix linkTransform;

				//	Transform : cluster transform
				cluster->GetTransformMatrix(transform);
				//	Link Transform : bone transform
				cluster->GetTransformLinkMatrix(linkTransform);

				boneDatas[boneIndex]->LocalTransform = FbxUtility::ToMatrix(transform);
				boneDatas[boneIndex]->GlobalTransform = FbxUtility::ToMatrix(linkTransform);

				for (int indexCount = 0; indexCount < cluster->GetControlPointIndicesCount(); indexCount++)
				{
					int temp = cluster->GetControlPointIndices()[indexCount];
					//	Bone weights of the cluster
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

			vector<VertexTextureNormalTangentBlend> vertices;
			for (FbxVertex* temp : gather)
			{
				VertexTextureNormalTangentBlend vertex;
				vertex = temp->Vertex;

				vertices.push_back(vertex);
			}

			meshPart->StartVertex = meshData->OutVertices.size();
			meshPart->VertexCount = vertices.size();

			meshData->OutVertices.insert(meshData->OutVertices.end(), vertices.begin(), vertices.end());
			meshData->MeshParts.push_back(meshPart);
		}
	}//for(MeshData)
}

void FbxLoader::WriteMesh(wstring saveFolder, wstring fileName)
{
	//	Create a folder
	Path::CreateFolder(saveFolder);

	BinaryWriter* w = new BinaryWriter();
	
	w->Open(saveFolder + fileName);

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

		w->UInt(meshData->OutVertices.size());
		w->Byte(&meshData->OutVertices[0], sizeof(VertexTextureNormalTangentBlend) * meshData->OutVertices.size());

		w->UInt(meshData->MeshParts.size());
		for (FbxMeshPartData* part : meshData->MeshParts)
		{
			w->String(part->MaterialName);

			w->UInt(part->StartVertex);
			w->UInt(part->VertexCount);

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
	//	GetAnimStackCount : The number of animation clips
	for (int i = 0; i < importer->GetAnimStackCount(); i++)
	{
		//	GetTakeInfo : Get the clip information
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

	//	GetGlobalSettings : Get all settings in a current scene
	FbxTime::EMode mode = scene->GetGlobalSettings().GetTimeMode();

	FbxClip *clip = new FbxClip();
	clip->FrameRate = (float)FbxTime::GetFrameRate(mode);

	FbxTakeInfo *takeInfo = importer->GetTakeInfo(index);
	clip->Name = takeInfo->mName.Buffer();

	//	Time of the current animation clip
	FbxTimeSpan span = takeInfo->mLocalTimeSpan;
	FbxTime tempDuration = span.GetDuration();

	int start = (int)span.GetStart().GetFrameCount();		//	clip start time
	int stop = (int)span.GetStop().GetFrameCount();			//	clip end time
	float duration = (float)tempDuration.GetMilliSeconds();	//	clip duration time

	if (start < stop)
		ReadKeyframeData(clip, scene->GetRootNode(), start, stop, -1, -1);

	clip->Duration = duration;
	clip->FrameCount = (stop - start) + 1;

	return clip;
}

void FbxLoader::ReadKeyframeData(struct FbxClip* clip, FbxNode* node, int start, int stop, int index, int parent)
{
	FbxNodeAttribute* attribute = node->GetNodeAttribute();
	if (attribute != NULL)
	{
		FbxNodeAttribute::EType nodeType = attribute->GetAttributeType();
		
		if (nodeType == FbxNodeAttribute::eSkeleton)
		{
			FbxKeyframe* keyframe = new FbxKeyframe();
			keyframe->BoneName = node->GetName();
			keyframe->index = index;
			keyframe->parent = parent;

			for (int i = start; i <= stop; i++)
			{
				//   SetFrame : Set time in frame format
				FbxTime animationTime;
				animationTime.SetFrame(i);
				
				FbxAMatrix matrix = node->EvaluateLocalTransform(animationTime);
				D3DXMATRIX transform = FbxUtility::ToMatrix(matrix);

				FbxKeyframeData data;
				data.Time = (float)animationTime.GetMilliSeconds();
				D3DXMatrixDecompose(&data.Scale, &data.Rotation, &data.Translation, &transform);

				keyframe->Transforms.push_back(data);
			}

			clip->KeyFrames.push_back(keyframe);
		}//if(nodeType)
	}

	for (int i = 0; i < node->GetChildCount(); i++)
		ReadKeyframeData(clip, node->GetChild(i), start, stop, clip->KeyFrames.size(), index);
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
	SAFE_DELETE(clip);

	w->Close();
	SAFE_DELETE(w);
}