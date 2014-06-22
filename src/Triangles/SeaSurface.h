#pragma once
#include "Object3D.h"

class SeaSurface : public Object3D
{
public:
	SeaSurface(float sideSize = 20.0);
	~SeaSurface(void);
private:
	void InitData();
	void GenerateData();
};

