#pragma once
#include "Object3D.h"
#include "LandSurface.h"
#include <list>

class List3D
{
protected:
	std::list<Object3D*> landscapeElements;	
public:
	List3D(void);
	~List3D(void);
	void Add(Object3D* o3d);
	virtual void InitGLBuffers(unsigned int programId);
	virtual void Draw();
	void DrawReflection();
};

class LandList3D : public List3D
{
private:
	unsigned shaderProgramId;
	float maxHeight;
public:
	LandList3D(void);
	~LandList3D(void);
	void Add(LandSurface* o3d);
	void InitGLBuffers(unsigned int programId);
	void Draw();
};

