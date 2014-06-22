#include "List3D.h"
#include <GL/glew.h>


List3D::List3D(void)
{
}

List3D::~List3D(void)
{
	for (std::list<Object3D*>::const_iterator citer = landscapeElements.begin(); citer != landscapeElements.end(); citer++)
		delete (*citer);
}

void List3D::Add(Object3D* o3d)
{
	landscapeElements.push_back(o3d);
}

void List3D::InitGLBuffers(unsigned int programId)
{
	for (std::list<Object3D*>::const_iterator citer = landscapeElements.begin(); citer != landscapeElements.end(); citer++)
		(*citer)->InitGLBuffers(programId);
}

void List3D::Draw()
{
	for (std::list<Object3D*>::const_iterator citer = landscapeElements.begin(); citer != landscapeElements.end(); citer++)
		(*citer)->Draw();
}

void List3D::DrawReflection()
{
	for (std::list<Object3D*>::const_iterator citer = landscapeElements.begin(); citer != landscapeElements.end(); citer++)
		(*citer)->DrawReflection();
}

LandList3D::LandList3D(void)
{
	maxHeight = -1;
}


LandList3D::~LandList3D(void)
{
	for (std::list<Object3D*>::const_iterator citer = landscapeElements.begin(); citer != landscapeElements.end(); citer++)
		delete (*citer);
}

void LandList3D::Add(LandSurface* ls)
{
	float landHeight = ls->GetMaxHeight();
	if (landHeight > maxHeight)
		maxHeight = landHeight;
	List3D::Add(ls);
}

void LandList3D::InitGLBuffers(unsigned int programId)
{
	shaderProgramId = programId;
	List3D::InitGLBuffers(programId);
}

void LandList3D::Draw()
{
	int lsType = glGetUniformLocation(shaderProgramId, "landscapeType");
	if (lsType > -1)
		glUniform1i(lsType, LandSurface::GetLandscapeType());
	int rType = glGetUniformLocation(shaderProgramId, "renderType");
	if (rType > -1)
		glUniform1i(rType, Object3D::RenderType::Object);
	int mh = glGetUniformLocation(shaderProgramId, "maxHeight");
	if (mh > -1)
		glUniform1f(mh, maxHeight);
	int md = glGetUniformLocation(shaderProgramId, "maxDepth");
	if (md > -1)
		glUniform1f(md, LandSurface::GetDeepening());

	for (std::list<Object3D*>::const_iterator citer = landscapeElements.begin(); citer != landscapeElements.end(); citer++)
		(*citer)->DrawTyped();
}