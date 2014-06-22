#include "Object3D.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

const char* Object3D::PosName = "pos";
const char* Object3D::NorName = "nor";
const char* Object3D::TexName = "tex";
const char* Object3D::ClrName = "clr";

Object3D::Object3D(void):
	dataCount(0),
	indexesCount(0),
	pData(0),
	pIndexes(0)
{
	size = glm::vec2(1.0);
	center = glm::vec3(0.0);
	landscapeType = 0;
	renderType = Object;
}

Object3D::~Object3D(void)
{
	glDeleteBuffers(2,vbo);
	glDeleteVertexArrays(1,&vao);
	if (pData)
	{
		delete[] pData;
		delete[] pIndexes;
	}
}

void Object3D::CreateArrays()
{
	if (pData)
	{
		delete[] pData;
		delete[] pIndexes;
	}
	pData = new VertexData [dataCount];
	pIndexes = new unsigned int [indexesCount];
}

void Object3D::InitGLBuffers(unsigned int programId)
{
	InitGLBuffers(programId, PosName, NorName, TexName, ClrName);
}

void Object3D::InitGLBuffers(GLuint programId, const char* posName,const char* norName, const char* texName, const char* clrName)
{
	shaderProgramId = programId;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray ( vao );
	
	glGenBuffers ( 2, &vbo[0]);
	
	glBindBuffer ( GL_ARRAY_BUFFER, vbo[0] );
	glBufferData ( GL_ARRAY_BUFFER, dataCount*sizeof(VertexData), pData, GL_STATIC_DRAW );
		
	glEnable(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, vbo[1] );
	glBufferData ( GL_ELEMENT_ARRAY_BUFFER, indexesCount*sizeof(unsigned int), pIndexes, GL_STATIC_DRAW );
	
	int	loc = glGetAttribLocation(programId, posName);
	if (loc>-1)
	{
		glVertexAttribPointer(loc,3,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)0);
		glEnableVertexAttribArray (loc);
	}
	int loc2 = glGetAttribLocation(programId, norName);
	if (loc2>-1)
	{
		glVertexAttribPointer(loc2,3,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)(0+sizeof(float)*3));
		glEnableVertexAttribArray (loc2);
	}
	/*int loc3 = glGetAttribLocation(programId, texName);
	if (loc3>-1)
	{
		glVertexAttribPointer(loc3,2,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)(0+sizeof(float)*6));
		glEnableVertexAttribArray (loc3);
	}
	int loc4 = glGetAttribLocation(programId, clrName);
	if (loc4>-1)
	{
		glVertexAttribPointer(loc4,3,GL_FLOAT,GL_FALSE,sizeof(VertexData),(GLuint *)(0+sizeof(float)*8));
		glEnableVertexAttribArray (loc4);
	}*/
	glBindVertexArray(0);	
}

void Object3D::Draw()
{
	int lsType = glGetUniformLocation(shaderProgramId, "landscapeType");
	if (lsType > -1)
		glUniform1i(lsType, landscapeType);
	int rType = glGetUniformLocation(shaderProgramId, "renderType");
	if (rType > -1)
		glUniform1i(rType, renderType);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES,indexesCount,GL_UNSIGNED_INT,0);
	glBindVertexArray(0);
}

void Object3D::DrawTyped()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES,indexesCount,GL_UNSIGNED_INT,0);
	glBindVertexArray(0);
}

void Object3D::DrawReflection()
{
	RenderType saveType = renderType;
	renderType = Reflection;
	Draw();
	renderType = saveType;
}
