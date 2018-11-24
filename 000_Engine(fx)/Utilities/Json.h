#pragma once

////////////////////////////////////
//	텍스트 파일 형식을 저장하기 위함

//	<텍스트 포맷 종류>
//	ini : 게임에서는 잘 안씀, 시스템에서 많이 사용
//	XML : 옛날에 많이 사용함
//	JSON : 가볍기 때문에 요즘은 다 JSON을 사용하는 추세이다.
//	=> 모두 형식 포맷을 가지고 있음

//	지금 프레임워크에는 XML과 JSON이 둘다 있다.
//	=> 이유는. 필요한 데이터가 XML일때 JSON파일로 변경 하기 위해서 이다.
////////////////////////////////////

namespace Json
{
	void GetValue(Json::Value& parent, string name, bool& vec);
	void SetValue(Json::Value& parent, string name, bool& vec);

	void GetValue(Json::Value& parent, string name, float& vec);
	void SetValue(Json::Value& parent, string name, float& vec);

	void GetValue(Json::Value& parent, string name, D3DXVECTOR2& vec);
	void SetValue(Json::Value& parent, string name, D3DXVECTOR2& vec);

	void GetValue(Json::Value& parent, string name, D3DXVECTOR3& vec);
	void SetValue(Json::Value& parent, string name, D3DXVECTOR3& vec);

	void GetValue(Json::Value& parent, string name, D3DXCOLOR& vec);
	void SetValue(Json::Value& parent, string name, D3DXCOLOR& vec);

	void GetValue(Json::Value& parent, string name, string& value);
	void SetValue(Json::Value& parent, string name, string& value);


	void ReadData(wstring file, Json::Value* root);
	void WriteData(wstring file, Json::Value* root);
}