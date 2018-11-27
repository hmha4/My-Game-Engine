#pragma once

#include <Windows.h>
#include <assert.h>
#include <crtdbg.h>

//	STL
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <functional>
#include <iterator>
using namespace std;

//	Direct Write
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

//	Direct3D
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <d3dx11effect.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "effects11d.lib")

//	ImGui
#include <imgui.h>
#include <imguiDx11.h>
#pragma comment(lib, "imgui.lib")

//	Json Cpp(Text 파일 포맷)
#include <json/json.h>
#pragma comment(lib, "jsoncpp.lib")
#pragma warning( disable : 4996 )

//	DirectXTex(Texture)
#include <DirectXTex.h>
#pragma comment(lib, "directxtex.lib")

//	FBX SDK
#define FBXSDK_SHARED
#include <fbxsdk.h>
#pragma comment(lib, "libfbxsdk.lib")
using namespace fbxsdk;

const wstring Shaders = L"../_Shaders/";
const wstring Effects = L"../_Effects/";
const wstring Contents = L"../_Contents/";
const wstring Textures = L"../_Textures/";
const wstring Assets = L"../_Assets/";
const wstring Models = L"../_Models/";

#define SAFE_RELEASE(p){ if(p){ (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p){ if(p){ delete (p); (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p){ if(p){ delete [] (p); (p) = NULL; } }

#include "./Systems/D3D.h"
#include "./Systems/DirectWrite.h"
#include "./Systems/Keyboard.h"
#include "./Systems/Mouse.h"
#include "./Systems/Time.h"

#include "./Renders/VertexLayouts.h"
#include "./Renders/CBuffer.h"
#include "./Renders/Context.h"
#include "./Renders/Shader.h"
#include "./Renders/Texture.h"
#include "./Renders/Material.h"
#include "./Renders/CsResource.h"

#include "./Executes/Execute.h"

#include "./Utilities/Math.h"
#include "./Utilities/Json.h"
#include "./Utilities/String.h"
#include "./Utilities/Path.h"
#include "./Utilities/MyGui.h"

#include "./Boundings/Ray.h"
#include "./Boundings/BBox.h"
#include "./Boundings/BSphere.h"

#include "./Draw/DebugLine.h"
#include "./Draw/MeshCube.h"
#include "./Draw/MeshCylinder.h"
#include "./Draw/MeshGrid.h"
#include "./Draw/MeshSphere.h"
#include "./Draw/MeshQuad.h"

typedef VertexTextureNormalTangentBlend ModelVertexType;

#include "./Objects/GameRender.h"
#include "./Objects/GameModel.h"
#include "./Objects/GameAnimModel.h"

