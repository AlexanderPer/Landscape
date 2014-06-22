#pragma once
#include "Object3D.h"

class LandSurface : public Object3D
{
private:
	static const float Deepening;
	static const int LandscapeType;
	float** T;				// heights matrix
	unsigned int N;			// 2^N+1 = matrix dimension
	unsigned int M;			// matrix dimension
	float randHeightCoeff;		
	float heightCenter;		// height center mountain
	float infinityDepth;
	float maxHeight;
public:
	LandSurface(float centerX = 0.0f, float centerY = 0.0f, float centerHeight = 0.0f);
	~LandSurface(void);
	float GetMaxHeight() { return maxHeight; }
	static float GetDeepening() { return Deepening; }
	static int GetLandscapeType() { return LandscapeType; }
private:
	void InitData();
	void GenerateData();

	float GetHeight(int i, int j) { return T[i][j]; }
	float GetRandom() { return (2.0f * (float) rand() / RAND_MAX) - 1.0f; }

	// functions for generate height matrix
	float EdgeLength(int i1, int i2, int j1, int j2);
	void MidEdge(int i1, int i2, int j1, int j2);
	void MidSquare(int i1, int i2, int j1, int j2);
	void SubSquare(int i1, int i2, int j1, int j2);
	void CreateaHeightMatrix();

	// for landscape polygonal representation 
	void InitPolygonalData();
	void GeneratePolygonalData();
};

