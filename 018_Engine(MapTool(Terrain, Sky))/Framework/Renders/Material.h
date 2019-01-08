#pragma once

namespace MaterialProperty
{
	struct ColorDesc
	{
		D3DXCOLOR   Ambient;      //   Ambient Color
		D3DXCOLOR   Diffuse;      //   Diffuse Color
		D3DXCOLOR   Specular;      //   Specular Color
		float      Shininess;      //   Specular Shininess(Exp)

		ColorDesc()
		{
			Ambient = D3DXCOLOR(1, 1, 1, 1);
			Diffuse = D3DXCOLOR(1, 1, 1, 1);
			Specular = D3DXCOLOR(0.9f, 0.9f, 0.9f, 16.0f);
			Shininess = 1;
		}
	};

	struct MaterialMap
	{
		Texture      *diffuseMap;   //   Diffuse Texture
		Texture      *specularMap;   //   Specular Texture
		Texture      *normalMap;      //   Normal Texture
		Texture      *detailMap;      //   Detail Texture

		MaterialMap()
		{
			diffuseMap = NULL;
			specularMap = NULL;
			normalMap = NULL;
			detailMap = NULL;
		}
	};
}

class Material
{
public:
	Material();
	Material(wstring shaderFile);
	Material(Shader * shader);
	~Material();

	// --------------------------------------------------------------------- //
	//  Name
	// --------------------------------------------------------------------- //
	void Name(wstring val) { name = val; }   //  Set the name of the Material
	wstring Name() { return name; }         //  Set the name of the Material

	// --------------------------------------------------------------------- //
	//  Shader
	// --------------------------------------------------------------------- //
	Shader *GetShader() { return shader; }   //  Get the shader file
	void SetShader(wstring file);         //  Set the shader file(wstring)
	void SetShader(string file);         //  Set the shader file(string)
	void SetShader(Shader * shader);

	// --------------------------------------------------------------------- //
	//  Color
	// --------------------------------------------------------------------- //
	D3DXCOLOR GetAmbient() { return color->Ambient; }         //   Get ambient color
	void SetAmbient(D3DXCOLOR& color);                     //   Set ambient color
	void SetAmbient(float r, float g, float b, float a = 1.0f);   //   Set ambient color

	D3DXCOLOR GetDiffuse() { return color->Diffuse; }         //   Get diffuse color
	void SetDiffuse(D3DXCOLOR& color);                     //   Set diffuse color
	void SetDiffuse(float r, float g, float b, float a = 1.0f);   //   Set diffuse color

	D3DXCOLOR GetSpecular() { return color->Specular; }         //   Get specular color
	void SetSpecular(D3DXCOLOR& color);                     //   Set specular color
	void SetSpecular(float r, float g, float b, float a = 1.0f);//   Set specular color

	// --------------------------------------------------------------------- //
	//  Shininess
	// --------------------------------------------------------------------- //
	float GetShininess() { return color->Shininess; }    //   Get shiniess
	void SetShininess(float val);                        //   Set shiniess

	 // --------------------------------------------------------------------- //
	 //  Textures
	 // --------------------------------------------------------------------- //
	Texture *GetDiffuseMap() { return map->diffuseMap; }                     //   Get diffuse texture
	void SetDiffuseMap(string file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set diffuse texture
	void SetDiffuseMap(wstring file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set diffuse texture

	Texture *GetSpecularMap() { return map->specularMap; }                  //   Get specular texture
	void SetSpecularMap(string file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set specular texture
	void SetSpecularMap(wstring file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set specular texture

	Texture *GetNormalMap() { return map->normalMap; }                     //   Get normal texture
	void SetNormalMap(string file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set normal texture
	void SetNormalMap(wstring file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set normal texture

	Texture *GetDetailMap() { return map->detailMap; }                     //   Get detail texture
	void SetDetailMap(string file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set detail texture
	void SetDetailMap(wstring file, D3DX11_IMAGE_LOAD_INFO* info = NULL);   //   Set detail texture

	void ChangeMaterialDesc(MaterialProperty::ColorDesc*desc);
	void ChangeMaterialMap(MaterialProperty::MaterialMap*materialMap);
public:
	void Clone(void **clone);   //   Clone

private:
	bool      bErase;
	wstring      name;         //   Material Name

	Shader      *shader;      //   Shader file
private:
	MaterialProperty::ColorDesc*color;
	MaterialProperty::MaterialMap*map;
};