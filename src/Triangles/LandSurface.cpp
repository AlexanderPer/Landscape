#include "LandSurface.h"

const float LandSurface::Deepening = -0.02;
const int LandSurface::LandscapeType = 2;

LandSurface::LandSurface(float centerX, float centerY, float centerHeight)
{
	N = 7;
	M = (unsigned int)pow(2,N) + 1;

	center = glm::vec3(centerX, centerY, Deepening);
	
	randHeightCoeff = 0.25f;
	heightCenter = centerHeight;
	infinityDepth = -100.0f * glm::length(size);

	InitData();
	//InitPolygonalData();
	CreateArrays();
	CreateaHeightMatrix();
	GenerateData();
	//GeneratePolygonalData();
	landscapeType = GetLandscapeType();
}


LandSurface::~LandSurface(void)
{
	if (T)
	{
		for (unsigned i = 0; i < M; i++)
			delete [] T[i];
		delete[] T;
	}
}

void LandSurface::InitData()
{
	dataCount = M * M;
	unsigned int nTriangles = 2 * (M - 1) * (M - 1);
	indexesCount = 3*nTriangles;
}

void LandSurface::GenerateData()
{
	glm::vec2 step = size / float(M - 1);
	glm::vec2 start = glm::vec2(center.x - size.x / 2.0f, center.y - size.y / 2.0f);

	for (unsigned int i = 0; i < M; i++)
		for (unsigned int j = 0; j < M; j++)
		{
			pData[i*M + j].pos = glm::vec3(start.x + step.x * i, start.y + step.y * j, center.z + GetHeight(i, j));
			pData[i*M + j].nor = glm::vec3(0, 0, 0);
			//pData[i*M + j].tex = glm::vec2(0, 0);
			//pData[i*M + j].clr = glm::vec3(0.5, 0.5, 0.5);			
		}

	int v = 0;
	int vi1, vi2, vi3;
	glm::vec3 norm;
	for (unsigned int i = 0; i < M - 1; i++)
		for (unsigned int j = 0; j < M - 1; j++)
		{
			vi1 = M * i + j;	vi2 = M * i + (j + 1);		vi3 = M * (i + 1) + j;

			pIndexes[v++]	= vi1;
			pIndexes[v++]	= vi2;
			pIndexes[v++]	= vi3;

			norm = glm::cross((pData[vi1].pos - pData[vi3].pos), (pData[vi1].pos - pData[vi2].pos));

			pData[vi1].nor += norm;
			pData[vi2].nor += norm;
			pData[vi3].nor += norm;

			vi1 = M * (i + 1) + j;	vi2 = M * i + (j + 1);		vi3 = M * (i + 1) + (j+1);

			pIndexes[v++]	= vi1;
			pIndexes[v++]	= vi2;
			pIndexes[v++]	= vi3;

			norm = glm::cross((pData[vi1].pos - pData[vi3].pos), (pData[vi1].pos - pData[vi2].pos));

			pData[vi1].nor += norm;
			pData[vi2].nor += norm;
			pData[vi3].nor += norm;
		}

		for (unsigned int i = 0; i < M; i++)
			for (unsigned int j = 0; j < M; j++)
				pData[i*M + j].nor = glm::normalize(pData[i*M + j].nor);
}

float LandSurface::EdgeLength(int i1, int i2, int j1, int j2)
{
	float dh = T[i2][j2] - T[i1][j1];
	float dx = size.x * (i2 - i1) / (M - 1);
	float dy = size.y * (j2 - j1) / (M - 1);
	return sqrt(dh*dh + dx*dx + dy*dy);
}

void LandSurface::MidEdge(int i1, int i2, int j1, int j2)
{
	if (T[(i1 + i2) / 2][(j1 + j2) / 2] > infinityDepth/2) return;
	float len = EdgeLength(i1, i2, j1, j2);
	float mean = (T[i1][j1] + T[i2][j2]) / 2;
	T[(i1 + i2) / 2][(j1 + j2) / 2] = mean + randHeightCoeff * len * GetRandom();
}

void LandSurface::MidSquare(int i1, int i2, int j1, int j2)
{
	if (T[(i1 + i2) / 2][(j1 + j2) / 2] > infinityDepth/2) return;
	float len = (EdgeLength((i1 + i2) / 2, (i1 + i2) / 2, j1, j2) + EdgeLength(i1, i2, (j1 + j2) / 2, (j1 + j2) / 2)) / 2;
	float mean = (T[i1][j1] + T[i2][j2] + T[i1][j2] + T[i2][j1]) / 4;
	T[(i1 + i2) / 2][(j1 + j2) / 2] = mean + randHeightCoeff * len * GetRandom();
}

void LandSurface::SubSquare(int i1, int i2, int j1, int j2)
{
	if ((i2 - i1 < 2) || (j2 - j1 < 2))
		return;
	MidEdge(i1, i2, j1, j1);
	MidEdge(i1, i2, j2, j2);
	MidEdge(i1, i1, j1, j2);
	MidEdge(i2, i2, j1, j2);
	MidSquare(i1, i2, j1, j2);
	SubSquare(i1,				(i1 + i2) / 2,	j1,				(j1 + j2) / 2);
	SubSquare((i1 + i2) / 2,	i2,				j1,				(j1 + j2) / 2);
	SubSquare((i1 + i2) / 2,	i2,				(j1 + j2) / 2,	j2);
	SubSquare(i1,				(i1 + i2) / 2,	(j1 + j2) / 2 ,	j2);
}

void LandSurface::CreateaHeightMatrix()
{
	T = new float*[M];
	for (unsigned i = 0; i < M; i++)
		T[i] = new float[M];
	
	for (unsigned i = 1; i < M - 1; i++)
		for (unsigned j = 1; j < M - 1; j++)
			T[i][j] = infinityDepth;
	// landscape height (land part) edge is 0
	for (unsigned i = 0; i < M; i++)
	{
		T[i][0] = 0;
		T[i][M - 1] = 0;
		T[0][i] = 0;
		T[M - 1][i] = 0;
	}
	T[(M - 1) / 2][(M - 1) / 2] = heightCenter; // mountain in center
	SubSquare(0, M - 1, 0, M - 1);
	
	// find	max height
	maxHeight = T[0][0];
	for (unsigned i = 0; i < M; i++)
		for (unsigned j = 0; j < M; j++)
			if (T[i][j] > maxHeight)
				maxHeight = T[i][j];

	maxHeight += Deepening;
}

void LandSurface::InitPolygonalData()
{
	unsigned int nTriangles = 2 * (M - 1) * (M - 1);
	dataCount = indexesCount = 3*nTriangles;
}

void LandSurface::GeneratePolygonalData()
{
	int v = 0;
	int trVerts = 0;
	int trVert1, trVert2, trVert3;
	//int vi1, vi2, vi3;
	glm::vec2 step = size / float(M - 1);
	glm::vec3 norm;
	for (unsigned int i = 0; i < M - 1; i++)
		for (unsigned int j = 0; j < M - 1; j++)
		{
			//vi1 = M * i + j;	vi2 = M * (i + 1) + j;	vi3 = M * i + (j + 1);			
			pIndexes[v++]	= trVert1 = trVerts++;
			pIndexes[v++]	= trVert2 = trVerts++;
			pIndexes[v++]	= trVert3 = trVerts++;

			pData[trVert1].pos = glm::vec3(step.x * i,			step.y * j,			GetHeight(i, j));
			pData[trVert2].pos = glm::vec3(step.x * (i + 1),	step.y * j,			GetHeight(i + 1, j));
			pData[trVert3].pos = glm::vec3(step.x * i,			step.y * (j + 1),	GetHeight(i, j + 1));
			norm = glm::normalize(glm::cross((pData[trVert1].pos - pData[trVert2].pos), (pData[trVert1].pos - pData[trVert3].pos)));
			pData[trVert1].nor = norm;
			pData[trVert2].nor = norm;
			pData[trVert3].nor = norm;			

			//vi1 = M * (i + 1) + j;	vi2 = M * (i + 1) + (j + 1);	vi3 = M * i + (j + 1);
			pIndexes[v++]	= trVert1 = trVerts++;
			pIndexes[v++]	= trVert2 = trVerts++;
			pIndexes[v++]	= trVert3 = trVerts++;

			pData[trVert1].pos = glm::vec3(step.x * (i + 1),	step.y * j,			GetHeight(i + 1, j));
			pData[trVert2].pos = glm::vec3(step.x * (i + 1),	step.y * (j + 1),	GetHeight(i + 1, j + 1));
			pData[trVert3].pos = glm::vec3(step.x * i,			step.y * (j + 1),	GetHeight(i, j + 1));

			norm = glm::normalize(glm::cross((pData[trVert1].pos - pData[trVert2].pos), (pData[trVert1].pos - pData[trVert3].pos)));

			pData[trVert1].nor = norm;
			pData[trVert2].nor = norm;
			pData[trVert3].nor = norm;
		}
		/*for (unsigned int i = 0; i < indexesCount; i++)
		{
			pData[i].tex = glm::vec2(0, 0);
			pData[i].clr = glm::vec3(0.5, 0.5, 0.5);
		}*/
}
