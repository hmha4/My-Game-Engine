#include "Framework.h"
#include "Material.h"

Material::Material()
	: shader(NULL)
	, bErase(true)
{
}

Material::Material(wstring shaderFile)
	: shader(NULL)
	, bErase(true)
{
	assert(shaderFile.length() > 0);


	SetShader(shaderFile);
}

Material::Material(Shader * shader)
	: shader(NULL)
	, bErase(false)
{
	//assert(shader != NULL);


	SetShader(shader);
}

Material::~Material()
{
	if (bErase == true)
		SAFE_DELETE(shader);
}

void Material::SetShader(wstring file)
{
	SAFE_DELETE(shader);

	if (file.length() > 0)
	{
		shader = new Shader(file);
	}
}

void Material::SetShader(string file)
{
	SetShader(String::ToWString(file));
}

void Material::SetShader(Shader * shader)
{
	if (bErase == true)
		SAFE_DELETE(this->shader);

	//SAFE_DELETE(buffer);

	this->shader = shader;

	bErase = false;
}


void Material::SetDiffuseMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDiffuseMap(String::ToWString(file), info);
}

void Material::SetDiffuseMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (map->diffuseMap != NULL)
	{
		if (map->diffuseMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(map->diffuseMap);
	map->diffuseMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("DiffuseMap")->SetResource(map->diffuseMap->SRV());
}

void Material::SetSpecularMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetSpecularMap(String::ToWString(file), info);
}

void Material::SetSpecularMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (map->specularMap != NULL)
	{
		if (map->specularMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(map->specularMap);

	map->specularMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("SpecularMap")->SetResource(map->specularMap->SRV());
}

void Material::SetNormalMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetNormalMap(String::ToWString(file), info);
}

void Material::SetNormalMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (map->normalMap != NULL)
	{
		if (map->normalMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(map->normalMap);

	map->normalMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("NormalMap")->SetResource(map->normalMap->SRV());
}

void Material::SetDetailMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDetailMap(String::ToWString(file), info);
}

void Material::SetDetailMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	if (map->detailMap != NULL)
	{
		if (map->detailMap->GetFile() == file)
			return;
	}

	SAFE_DELETE(map->detailMap);

	map->detailMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("DetailMap")->SetResource(map->detailMap->SRV());
}

void Material::ChangeMaterialDesc(MaterialProperty::ColorDesc * desc)
{
	color = desc;
	shader->AsVector("Ambient")->SetFloatVector(color->Ambient);
	shader->AsVector("Diffuse")->SetFloatVector(color->Diffuse);
	shader->AsVector("Specular")->SetFloatVector(color->Specular);
	shader->AsScalar("Shininess")->SetFloat(color->Shininess);
}

void Material::ChangeMaterialMap(MaterialProperty::MaterialMap * materialMap)
{
	map = materialMap;

	if (map->diffuseMap != NULL)
		shader->AsShaderResource("DiffuseMap")->SetResource(map->diffuseMap->SRV());
	if (map->specularMap != NULL)
		shader->AsShaderResource("SpecularMap")->SetResource(map->specularMap->SRV());
	if (map->normalMap != NULL)
		shader->AsShaderResource("NormalMap")->SetResource(map->normalMap->SRV());
	if (map->detailMap != NULL)
		shader->AsShaderResource("DetailMap")->SetResource(map->detailMap->SRV());
}

void Material::Clone(void ** clone)
{
	Material *material = new Material();
	material->name = this->name;

	if (this->shader != NULL)
		material->SetShader(this->shader->GetFile());

	/*material->SetAmbient(this->GetAmbient());
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
	material->SetDetailMap(this->detailMap->GetFile());*/

	*clone = material;
}

void Material::SetAmbient(D3DXCOLOR & color)
{
	if (this->color->Ambient == color)
		return;

	this->color->Ambient = color;
}

void Material::SetAmbient(float r, float g, float b, float a)
{
	SetAmbient(D3DXCOLOR(r, g, b, a));
}

void Material::SetDiffuse(D3DXCOLOR & color)
{
	if (this->color->Diffuse == color)
		return;

	this->color->Diffuse = color;
}

void Material::SetDiffuse(float r, float g, float b, float a)
{
	SetDiffuse(D3DXCOLOR(r, g, b, a));
}

void Material::SetSpecular(D3DXCOLOR & color)
{
	if (this->color->Specular == color)
		return;

	this->color->Specular = color;
}

void Material::SetSpecular(float r, float g, float b, float a)
{
	SetSpecular(D3DXCOLOR(r, g, b, a));
}

void Material::SetShininess(float val)
{
	if (this->color->Shininess == val)
		return;

	this->color->Shininess = val;
}