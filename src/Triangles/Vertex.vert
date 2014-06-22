//shader version
#version 150 core

uniform mat4 modelViewMatrix;

//inverse and transpose matrix for normals
uniform mat4 normalMatrix;

//projectionMatrix*modelViewMatrix
uniform mat4 modelViewProjectionMatrix;

// current drawing render type (Object = 0, Reflection = 1, Shadow = 2)
uniform int renderType;
int Object = 0;
int Reflection = 1;

//input vertex: position, normal, texture coordinates
in vec3 pos;
in vec3 nor;
//in vec2 tex;
//in vec3 clr;

//output vertex to fragment shader
out VertexData
{
	vec3 position;
	vec3 normal;
	//vec2 texcoord;
	//vec3 color;
	vec3 modelnor;
	vec3 modelpos;
	//float height;
} VertexOut;

void main()
{
	if (renderType == Object)
		gl_Position = modelViewProjectionMatrix * vec4(pos.xyz,1);
	else if (renderType == Reflection)
	{
		gl_Position = modelViewProjectionMatrix * vec4(pos.x, pos.y, -pos.z, 1);
	}

	VertexOut.position = vec3(modelViewMatrix*vec4(pos.xyz,1));
	VertexOut.normal = vec3(normalMatrix*vec4(nor.xyz,1));	
	//VertexOut.texcoord = tex;
	//VertexOut.color = clr;
	VertexOut.modelnor = nor;
	VertexOut.modelpos = pos;
	//VertexOut.height = pos.z;
}
