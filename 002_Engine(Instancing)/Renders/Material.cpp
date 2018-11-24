#include "stdafx.h"
#include "Material.h"

Material::Material()
	: shader(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL), detailMap(NULL)
	, buffer(NULL)
{
	color.Ambient = D3DXCOLOR(0, 0, 0, 1);
	color.Diffuse = D3DXCOLOR(0, 0, 0, 1);
	color.Specular = D3DXCOLOR(0, 0, 0, 1);
	color.Shininess = 0.0f;
}

Material::Material(wstring shaderFile)
	: diffuseMap(NULL),specularMap(NULL), normalMap(NULL),detailMap(NULL)
{
	assert(shaderFile.length() > 0);
	
	color.Ambient = D3DXCOLOR(0, 0, 0, 1);
	color.Diffuse = D3DXCOLOR(0, 0, 0, 1);
	color.Specular = D3DXCOLOR(0, 0, 0, 1);
	color.Shininess = 0.0f;

	SetShader(shaderFile);
}

Material::~Material()
{
	SAFE_DELETE(shader);
	SAFE_DELETE(buffer);

	SAFE_DELETE(diffuseMap);
	SAFE_DELETE(specularMap);
	SAFE_DELETE(normalMap);
	SAFE_DELETE(detailMap);
}

void Material::SetShader(wstring file)
{
	SAFE_DELETE(shader);
	SAFE_DELETE(buffer);

	if (file.length() > 0)
	{
		shader = new Shader(file);
		buffer = new CBuffer(shader, "CB_Material", &color, sizeof(ColorDesc));

		if (diffuseMap != NULL)
			shader->AsShaderResource("DiffuseMap")->SetResource(diffuseMap->SRV());
		if (specularMap != NULL)
			shader->AsShaderResource("SpecularMap")->SetResource(specularMap->SRV());
		if (normalMap != NULL)
			shader->AsShaderResource("NormalMap")->SetResource(normalMap->SRV());
		if (detailMap != NULL)
			shader->AsShaderResource("DetailMap")->SetResource(detailMap->SRV());
	}
}

void Material::SetShader(string file)
{
	SetShader(String::ToWString(file));
}

void Material::SetDiffuseMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDiffuseMap(String::ToWString(file), info);
}

void Material::SetDiffuseMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SAFE_DELETE(diffuseMap);
	
	diffuseMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("DiffuseMap")->SetResource(diffuseMap->SRV());
}

void Material::SetSpecularMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetSpecularMap(String::ToWString(file), info);
}

void Material::SetSpecularMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SAFE_DELETE(specularMap);

	specularMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("SpecularMap")->SetResource(specularMap->SRV());
}

void Material::SetNormalMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetNormalMap(String::ToWString(file), info);
}

void Material::SetNormalMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SAFE_DELETE(normalMap);

	normalMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("NormalMap")->SetResource(normalMap->SRV());
}

void Material::SetDetailMap(string file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SetDetailMap(String::ToWString(file), info);
}

void Material::SetDetailMap(wstring file, D3DX11_IMAGE_LOAD_INFO * info)
{
	SAFE_DELETE(detailMap);

	detailMap = new Texture(file, info);

	if (shader != NULL)
		shader->AsShaderResource("DetailMap")->SetResource(detailMap->SRV());
}

void Material::Clone(void ** clone)
{
	Material *material = new Material();
	material->name = this->name;

	if (this->shader != NULL)
		material->SetShader(this->shader->GetFile());
	
	material->SetAmbient(*this->GetAmbient());
	material->SetDiffuse(*this->GetDiffuse());
	material->SetSpecular(*this->GetSpecular());
	material->SetShininess(*this->GetShininess());

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
	this->color.Ambient = color; 

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetAmbient(float r, float g, float b, float a)	
{
	color.Ambient = D3DXCOLOR(r, g, b, a);

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetDiffuse(D3DXCOLOR & color)
{ 
	this->color.Diffuse = color;

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetDiffuse(float r, float g, float b, float a)	
{
	color.Diffuse = D3DXCOLOR(r, g, b, a);

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetSpecular(D3DXCOLOR & color)
{
	this->color.Specular = color; 

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetSpecular(float r, float g, float b, float a)	
{
	color.Specular = D3DXCOLOR(r, g, b, a);

	if (buffer != NULL)
		buffer->Change();
}

void Material::SetShininess(float val) 
{ 
	color.Shininess = val; 

	if (buffer != NULL)
		buffer->Change();
}
