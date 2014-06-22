#pragma once
#include <glm/glm.hpp>

//helper struct for Vertex
//contains position, normal and texture coordinates
struct VertexData
{
	 glm::vec3 pos;
	 glm::vec3 nor;
	 //glm::vec2 tex;
	 //glm::vec3 clr;
};

//base class for 3D objects
class Object3D
{	
private:
	static const char* PosName;
	static const char* NorName;
	static const char* TexName;
	static const char* ClrName;	
	unsigned int vbo[2];		//VertexBufferObject one for MeshVertexData, another for Indexes
	unsigned int vao;			//one VertexArrayObject
	unsigned int shaderProgramId;	
protected:
	VertexData* pData;			//pointer to object's internal data
	unsigned int dataCount;

	unsigned int* pIndexes;		//pointer to indexes (list of vetrices) 
	unsigned int indexesCount;

	glm::vec2 size;				//XY object size
	glm::vec3 center;
	unsigned landscapeType;
public:
	enum RenderType { Object = 0, Reflection = 1, Shadow = 2 };
	Object3D(void);
	virtual ~Object3D(void);	
	void InitGLBuffers(unsigned int programId); 
	virtual void Draw();
	virtual void DrawTyped();
	virtual void DrawReflection();
protected:
	void CreateArrays();
private:
	RenderType renderType;
	void InitGLBuffers(unsigned int programId, const char* posName, const char* norName, const char* texName, const char* clrName);
};

