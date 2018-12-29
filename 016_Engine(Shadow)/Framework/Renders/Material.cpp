#include "Framework.h"
#include "Material.h"

Material::Material()
	: shader(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL), detailMap(NULL)
	, buffer(NULL), bErase(true)
{
	color.Ambient = D3DXCOLOR(0, 0, 0, 1);
	color.Diffuse = D3DXCOLOR(0, 0, 0, 1);
	color.Specular = D3DXCOLOR(0, 0, 0, 1);
	color.Shininess = 0.0f;
}

Material::Material(wstring shaderFile)
	: shader(NULL)
	, diffuseMap(NULL),specularMap(NULL), normalMap(NULL),detailMap(NULL)
	, buffer(NULL), bErase(true)
{
	assert(shaderFile.length() > 0);
	
	color.Ambient = D3DXCOLOR(0, 0, 0, 1);
	color.Diffuse = D3DXCOLOR(0, 0, 0, 1);
	color.Specular = D3DXCOLOR(0, 0, 0, 1);
	color.Shininess = 0.0f;

	SetShader(shaderFile);
}

Material::Material(Shader * shader)
	: shader(NULL)
	, diffuseMap(NULL), specularMap(NULL), normalMap(NULL), detailMap(NULL)
	, buffer(NULL), bErase(false)
{
	//assert(shader != NULL);

	color.Ambient = D3DXCOLOR(0, 0, 0, 1);
	color.Diffuse = D3DXCOLOR(0, 0, 0, 1);
	color.Specular = D3DXCOLOR(0, 0, 0, 1);
	color.Shininess = 0.0f;

	SetShader(shader);
}

Material::~Material()
{
	if(bErase == true)
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

void Material::SetShader(Shader * shader)
{
	if(bErase == true)
		SAFE_DELETE(this->shader);

	if(buffer != NULL)
		Context::Get()->Erase(buffer);
	//SAFE_DELETE(buffer);

	this->shader = shader;

	bErase = false;

	if (this->shader != NULL)
	{
		buffer = new CBuffer(this->shader, "CB_Material", &color, sizeof(ColorDesc));

		if (diffuseMap != NULL)
			this->shader->AsShaderResource("DiffuseMap")->SetResource(diffuseMap->SRV());
		if (specularMap != NULL)
			this->shader->AsShaderResource("SpecularMap")->SetResource(specularMap->SRV());
		if (normalMap != NULL)
			this->shader->AsShaderResource("NormalMap")->SetResource(normalMap->SRV());
		if (detailMap != NULL)
			this->shader->AsShaderResource("DetailMap")->SetResource(detailMap->SRV());
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

	if (shader != NULL)
		shader->AsShaderResource("DiffuseMap")->SetResource(diffuseMap->SRV());
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

	if (shader != NULL)
		shader->AsShaderResource("SpecularMap")->SetResource(specularMap->SRV());
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

	if (shader != NULL)
		shader->AsShaderResource("NormalMap")->SetResource(normalMap->SRV());
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

	if (shader != NULL)
		shader->AsShaderResource("DetailMap")->SetResource(detailMap->SRV());
}

void Material::Clone(void ** clone)
{
	Material *material = new Material();
	material->name = this->name;

	if (this->shader != NULL)
		material->SetShader(this->shader->GetFile());
	
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
	if (this->color.Ambient == color || buffer == NULL)
		return;

	this->color.Ambient = color;
	buffer->Change();
}

void Material::SetAmbient(float r, float g, float b, float a)	
{
	SetAmbient(D3DXCOLOR(r, g, b, a));
}

void Material::SetDiffuse(D3DXCOLOR & color)
{ 
	if (this->color.Diffuse == color || buffer == NULL)
		return;

	this->color.Diffuse = color;
	buffer->Change();
}

void Material::SetDiffuse(float r, float g, float b, float a)	
{
	SetDiffuse(D3DXCOLOR(r, g, b, a));
}

void Material::SetSpecular(D3DXCOLOR & color)
{
	if (this->color.Specular == color || buffer == NULL)
		return;

	this->color.Specular = color;
	buffer->Change();
}

void Material::SetSpecular(float r, float g, float b, float a)	
{
	SetSpecular(D3DXCOLOR(r, g, b, a));
}

void Material::SetShininess(float val) 
{ 
	if (this->color.Shininess == val || buffer == NULL)
		return;

	this->color.Shininess = val;
	buffer->Change();
}
