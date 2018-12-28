#pragma once

class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void Update() = 0;
	virtual void LookAtPosition(D3DXVECTOR3* vec) {}
	virtual void LookAtPosition(float x, float y, float z) {}
	virtual void CameraType(wstring camType) {}
	virtual wstring CameraType() { return L""; }
	virtual void Speed(float val) {}
	virtual float Speed() { return 0.0f; }

	void Position(D3DXVECTOR3* vec)
	{
		*vec = position;
	}
	void Position(float x, float y, float z)
	{
		position = D3DXVECTOR3(x, y, z);
		View();
	}

	void Rotation(D3DXVECTOR2* vec)
	{
		*vec = rotation;
	}

	void RotationDegree(D3DXVECTOR2* vec)
	{
		*vec = rotation * 180.0f / (float)D3DX_PI;
	}

	void Rotation(float x, float y)
	{
		rotation = D3DXVECTOR2(x, y);

		Rotation();
	}

	void RotationDegree(float x, float y)
	{
		//rotation = D3DXVECTOR2(x, y) * (float)D3DX_PI / 180.0f;
		rotation = D3DXVECTOR2(x, y) * 0.01745328f;
		Rotation();
	}

	void MatrixView(D3DXMATRIX* view)
	{
		*view = matView;
	}
	void MatrixRotation(D3DXMATRIX* view)
	{
		*view = matRotation;
	}

	void Forward(D3DXVECTOR3* vec)
	{
		*vec = forward;
	}

	void Right(D3DXVECTOR3* vec)
	{
		*vec = right;
	}

	void Up(D3DXVECTOR3* vec)
	{
		*vec = up;
	}

protected:
	//	이동 처리
	virtual void Move();
	//	회전 처리
	virtual void Rotation();
	//	뷰 행렬 갱신
	virtual void View();


private:
	D3DXVECTOR3 position;
	D3DXVECTOR2 rotation;

	D3DXVECTOR3 forward;
	D3DXVECTOR3 right;
	D3DXVECTOR3 up;


	//	회전 행렬
	D3DXMATRIX matRotation;
	//	뷰 행렬(최종적으로 shader에 들어가 view)
	D3DXMATRIX matView;
};