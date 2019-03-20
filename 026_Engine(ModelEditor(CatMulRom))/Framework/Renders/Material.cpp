#include "Framework.h"
#include "Material.h"

Material::Material()
	: effect(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL), detailMap(NULL)
	, buffer(NULL), bErase(true)
{
	color.Ambient = D3DXCOLOR(1, 1, 1, 1);
	color.Diffuse = D3DXCOLOR(1, 1, 1, 1);
	color.Specular = D3DXCOLOR(0.9f, 0.9f, 0.9f, 16.0f);
	color.Shininess = 0.0f;
}

Material::Material(wstring effectFile)
	: effect(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL), detailMap(NULL)
	, buffer(NULL), bErase(true)
{
	assert(effectFile.length() > 0);

	color.Ambient = D3DXCOLOR(1, 1, 1, 1);
	color.Diffuse = D3DXCOLOR(1, 1, 1, 1);
	color.Specular = D3DXCOLOR(0.9f, 0.9f, 0.9f, 16.0f);
	color.Shininess = 0.0f;

	SetEffect(effectFile);
}

Material::Material(Effect * effect)
	: effect(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL), detailMap(NULL)
	, buffer(NULL), bErase(false)
{
	assert(effect == NULL);

	color.Ambient = D3DXCOLOR(1, 1, 1, 1);
	color.Diffuse = D3DXCOLOR(1, 1, 1, 1);
	color.Specular = D3DXCOLOR(0.9f, 0.9f, 0.9f, 16.0f);
	color.Shininess = 0.0f;

	SetEffect(effect);
}

Material::~Material()
{
	if (bErase == true)
		SAFE_DELETE(effect);

	if (buffer != NULL)
		Context::Get()->Erase(buffer);

	SAFE_DELETE(diffuseMap);
	SAFE_DELETE(specularMap);
	SAFE_DELETE(normalMap);
	SAFE_DELETE(detailMap);
}

void Material::SetEffect(wstring file)
{
	if (bErase == true)
		SAFE_DELETE(effect);
	if (buffer != NULL)
		Context::Get()->Erase(buffer);

	bErase = true;

	if (file.length() > 0)
	{
		effect = new Effect(file);
		worldVar = effect->AsMatrix("World");
		diffuseVar = effect->AsSRV("DiffuseMap");
		specularVar = effect->AsSRV("SpecularMap");
		normalVar = effect->AsSRV("NormalMap");
		detailVar = effect->AsSRV("DetailMap");

		buffer = new CBuffer(effect, "CB_Material", &color, sizeof(ColorDesc));

		if (diffuseMap != NULL)
			diffuseVar->SetResource(diffuseMap->SRV());
		if (specularMap != NULL)
			specularVar->SetResource(specularMap->SRV());
		if (normalMap != NULL)
			normalVar->SetResource(normalMap->SRV());
		if (detailMap != NULL)
			detailVar->SetResource(detailMap->SRV());
	}
}

void Material::SetEffect(string file)
{
	SetEffect(String::ToWString(file));
}

void Material::SetEffect(Effect * effect)
{
	if (bErase == true)
		SAFE_DELETE(this->effect);

	if (buffer != NULL)
		Context::Get()->Erase(buffer);
	//SAFE_DELETE(buffer);

	this->effect = effect;
	worldVar = effect->AsMatrix("World");
	diffuseVar = effect->AsSRV("DiffuseMap");
	specularVar = effect->AsSRV("SpecularMap");
	normalVar = effect->AsSRV("NormalMap");
	detailVar = effect->AsSRV("DetailMap");

	bErase = false;

	if (this->effect != NULL)
	{
		buffer = new CBuffer(this->effect, "CB_Material", &color, sizeof(ColorDesc));

		if (diffuseMap != NULL)
			diffuseVar->SetResource(diffuseMap->SRV());
		if (specularMap != NULL)
			specularVar->SetResource(specularMap->SRV());
		if (normalMap != NULL)
			normalVar->SetResource(normalMap->SRV());
		if (detailMap != NULL)
			detailVar->SetResource(detailMap->SRV());
	}
}

void Material::SetDiffuseMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDiffuseMap(String::ToWString(file), info);
}

void Material::SetDiffuseMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (diffuseMap != NULL)
	{
		if (diffuseMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(diffuseMap);
	diffuseMap = new Texture(file, info);

	if (effect != NULL)
		effect->AsSRV("DiffuseMap")->SetResource(diffuseMap->SRV());
}

void Material::SetSpecularMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetSpecularMap(String::ToWString(file), info);
}

void Material::SetSpecularMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (specularMap != NULL)
	{
		if (specularMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(specularMap);

	specularMap = new Texture(file, info);

	if (effect != NULL)
		effect->AsSRV("SpecularMap")->SetResource(specularMap->SRV());
}

void Material::SetNormalMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetNormalMap(String::ToWString(file), info);
}

void Material::SetNormalMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (normalMap != NULL)
	{
		if (normalMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(normalMap);

	normalMap = new Texture(file, info);

	if (effect != NULL)
		effect->AsSRV("NormalMap")->SetResource(normalMap->SRV());
}

void Material::SetDetailMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDetailMap(String::ToWString(file), info);
}

void Material::SetDetailMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (detailMap != NULL)
	{
		if (detailMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(detailMap);

	detailMap = new Texture(file, info);

	if (effect != NULL)
		effect->AsSRV("DetailMap")->SetResource(detailMap->SRV());
}

void Material::Clone(void ** clone)
{
	Material *material = new Material();
	material->name = this->name;

	if (this->effect != NULL)
		material->SetEffect(this->effect->GetFile());

	material->SetAmbient(this->GetAmbient());
	material->SetDiffuse(this->GetDiffuse());
	material->SetSpecular(this->GetSpecular());
	material->SetShininess(this->GetShininess());

	if (this->diffuseMap != NULL)
		material->SetDiffuseMap(this->diffuseMap->GetFile());
	if (this->specularMap != NULL)
		material->SetSpecularMap(this->specularMap->GetFile());
	if (this->normalMap != NULL)
		material->SetNormalMap(this->normalMap->GetFile());
	if (this->detailMap != NULL)
		material->SetDetailMap(this->detailMap->GetFile());

	*clone = material;
}

void Material::SetAmbient(D3DXCOLOR & color)
{
	if (this->color.Ambient == color)
		return;


	this->color.Ambient = color;

	if(buffer != NULL)
		buffer->Change();
}

void Material::SetAmbient(float r, float g, float b, float a)
{
	SetAmbient(D3DXCOLOR(r, g, b, a));
}

void Material::SetDiffuse(D3DXCOLOR & color)
{
	if (this->color.Diffuse == color)
		return;

	this->color.Diffuse = color;

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetDiffuse(float r, float g, float b, float a)
{
	SetDiffuse(D3DXCOLOR(r, g, b, a));
}

void Material::SetSpecular(D3DXCOLOR & color)
{
	if (this->color.Specular == color)
		return;

	this->color.Specular = color;

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetSpecular(float r, float g, float b, float a)
{
	SetSpecular(D3DXCOLOR(r, g, b, a));
}

void Material::SetShininess(float val)
{
	if (this->color.Shininess == val)
		return;

	this->color.Shininess = val;

	if (buffer != NULL)
		buffer->Change();
}