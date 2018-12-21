#include "stdafx.h"
#include "Json.h"
#include <fstream>

void Json::GetValue(Json::Value & parent, string name, bool & vec)
{
	vec = parent[name.c_str()].asBool();
}

void Json::SetValue(Json::Value & parent, string name, bool & vec)
{
	parent[name.c_str()] = vec;
}

void Json::GetValue(Json::Value & parent, string name, float & vec)
{
	vec = parent[name.c_str()].asFloat();
}

void Json::SetValue(Json::Value & parent, string name, float & vec)
{
	parent[name.c_str()] = vec;
}

void Json::GetValue(Json::Value & parent, string name, D3DXVECTOR2 & vec)
{
	for (int i = 0; i < 2; i++)
		vec[i] = parent[name.c_str()][i].asFloat();
}

void Json::SetValue(Json::Value & parent, string name, D3DXVECTOR2 & vec)
{
	for (int i = 0; i < 2; i++)
		parent[name.c_str()].append(vec[i]);
}

void Json::GetValue(Json::Value & parent, string name, D3DXVECTOR3 & vec)
{
	for (int i = 0; i < 3; i++)
		vec[i] = parent[name.c_str()][i].asFloat();
}

void Json::SetValue(Json::Value & parent, string name, D3DXVECTOR3 & vec)
{
	for (int i = 0; i < 3; i++)
		parent[name.c_str()].append(vec[i]);
}

void Json::GetValue(Json::Value & parent, string name, D3DXCOLOR & vec)
{
	for (int i = 0; i < 4; i++)
		vec[i] = parent[name.c_str()][i].asFloat();
}

void Json::SetValue(Json::Value & parent, string name, D3DXCOLOR & vec)
{
	for (int i = 0; i < 4; i++)
		parent[name.c_str()].append(vec[i]);
}

void Json::GetValue(Json::Value & parent, string name, string & value)
{
	value = "";

	for (size_t i = 0; i < parent[name.c_str()].size(); i++)
		value.append(parent[name.c_str()][i].asString());
}

void Json::SetValue(Json::Value & parent, string name, string & value)
{
	parent[name.c_str()].append(value);
}

void Json::ReadData(wstring file, Json::Value * root)
{
	ifstream stream;

	stream.open(file);
	{
		Json::Reader reader;
		reader.parse(stream, *root);
	}
	stream.close();
}

void Json::WriteData(wstring file, Json::Value * root)
{
	ofstream stream;

	string temp = String::ToString(file);
	stream.open(temp);
	{
		Json::StyledWriter writer;

		stream << writer.write(*root);
	}
	stream.close();
}
