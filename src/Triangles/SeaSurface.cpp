#include "SeaSurface.h"


SeaSurface::SeaSurface(float sideSize)
{
	size = glm::vec2(sideSize);
	InitData();
	CreateArrays();
	GenerateData();
	landscapeType = 1;
}

SeaSurface::~SeaSurface(void)
{
}

void SeaSurface::InitData()
{
	dataCount = 4; 
	unsigned int nTriangles = 2;
	indexesCount = 3*nTriangles;
}

void SeaSurface::GenerateData()
{
	for (unsigned int i=0; i<dataCount; i++)
	{
		pData[i].pos = glm::vec3(float((i % 2) ? center.x + size.x / 2 : center.x - size.x / 2), 
			float((i > 1) ? center.y + size.y / 2 : center.y - size.y / 2), 
			center.z);
		pData[i].nor = glm::vec3(0, 0, 1);
		//pData[i].tex = glm::vec2(float((i % 2) ? 1 : 0), float((i > 1) ? 1 : 0));
		//pData[i].clr = glm::vec3(float((i % 2) ? 1 : 0), float((i > 1) ? 1 : 0), 0);
	}
	pIndexes[0] = 0; pIndexes[1] = 1; pIndexes[2] = 3;
	pIndexes[3+0] = 0; pIndexes[3+1] = 2; pIndexes[3+2] = 3;	
}
