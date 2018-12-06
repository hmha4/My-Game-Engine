#include "stdafx.h"
#include "ExportMesh.h"
#include "../Fbx/Exporter.h"
#include "../Objects/GameModel.h"


ExportMesh::ExportMesh()
{
	Fbx::Exporter*exporter = NULL;

	////Capsule
	//exporter = new Fbx::Exporter(Assets + L"Meshes/Capsule.fbx");
	//exporter->ExportMaterial(Models2 + L"/Meshes/", L"Capsule.material");
	//exporter->ExportMesh(Models2 + L"/Meshes/", L"Capsule.mesh");

	////Cube
	//exporter = new Fbx::Exporter(Assets + L"Meshes/Cube.fbx");
	//exporter->ExportMaterial(Models2 + L"/Meshes/", L"Cube.material");
	//exporter->ExportMesh(Models2 + L"/Meshes/", L"Cube.mesh");

	////Cube
	//exporter = new Fbx::Exporter(Assets + L"Meshes/Cylinder.fbx");
	//exporter->ExportMaterial(Models2 + L"/Meshes/", L"Cylinder.material");
	//exporter->ExportMesh(Models2 + L"/Meshes/", L"Cylinder.mesh");

	////Plane
	//exporter = new Fbx::Exporter(Assets + L"Meshes/Quad.fbx");
	//exporter->ExportMaterial(Models2 + L"/Meshes/", L"Quad.material");
	//exporter->ExportMesh(Models2 + L"/Meshes/", L"Quad.mesh");
	//
	////Plane
	exporter = new Fbx::Exporter(Assets + L"Meshes/Sphere.fbx");
	exporter->ExportMaterial(Models + L"/Sphere/", L"Sphere.material");
	exporter->ExportMesh(Models + L"/Sphere/", L"Sphere.mesh");
	//
	////Plane
	//exporter = new Fbx::Exporter(Assets + L"Meshes/Plane.fbx");
	//exporter->ExportMaterial(Models2 + L"/Meshes/", L"Plane.material");
	//exporter->ExportMesh(Models2 + L"/Meshes/", L"Plane.mesh");

	//exporter = new Fbx::Exporter(Assets + L"Rabbit/stanford-bunny.fbx");
	//exporter->ExportMaterial(Models2 + L"/Rabbit/", L"stanford-bunny.material");
	//exporter->ExportMesh(Models2 + L"/Rabbit/", L"stanford-bunny.mesh");
	//SAFE_DELETE(exporter);



	////Tank
	//exporter = new Fbx::Exporter(Assets + L"Soldier/Soldier_demo.fbx");
	//exporter->ExportMaterial(Models2 + L"Soldier/", L"Soldier.material");
	//exporter->ExportMesh(Models2 + L"Soldier/", L"Soldier.mesh");

	//exporter = new Fbx::Exporter(Assets + L"Soldier/demo_combat_idle.FBX");
	//exporter->ExportAnimation(Models2 + L"Soldier/", L"Soldier_Idle.anim", 0);
	//SAFE_DELETE(exporter);

	//Model*model = new Model();
	//model->ReadMaterial(Models + L"/Tank/", L"Tank.material");
	//model->ReadMesh(Models + L"/Tank/", L"Tank.mesh");

}

ExportMesh::~ExportMesh()
{
}

void ExportMesh::Update()
{
}

void ExportMesh::PreRender()
{

}

void ExportMesh::Render()
{
}

void ExportMesh::PostRender()
{
	
}

void ExportMesh::ResizeScreen()
{
}

