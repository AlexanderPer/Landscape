//standard libraries
#include <iostream>
#include <ctime>
using namespace std;
//opengl headers
#include <GL/glew.h>
#include <GL/freeglut.h>

//opengl mathematics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>


//functions for shader compilation and linking
#include "shaderhelper.h"
//objects for drawing
#include "Object3D.h"
#include "SeaSurface.h"
#include "LandSurface.h"
#include "List3D.h"

//models for drawing:
LandList3D land;
List3D sea;

//struct for loading shaders
ShaderProgram shaderProgram;

//window size
int windowWidth = 800;
int windowHeight = 600;

//last mouse coordinates
int mouseX = 0, mouseY = 0;

//for (x, y) orientation
//camera position
glm::vec3 eye(-1, -1, 0.3);
//reference point position
glm::vec3 cen(0.0, 0.0, 0.0);
//up vector direction (head of observer)
glm::vec3 up(0, 0, 1);

glm::vec3 lightpos(-50, 0, 10);

//matrices
glm::mat4x4 modelViewMatrix;
glm::mat4x4 projectionMatrix;
glm::mat4x4 modelViewProjectionMatrix;
glm::mat4x4 normalMatrix;

///defines drawing mode
bool useTexture = true;

int timePrev;
bool moveForward = false;
bool moveBackward = false;
int moveRight = 0;
glm::mediump_ivec3 camMoveDir(0);

//texture identificator
GLuint texId[1];

//names of shader files. program will search for them during execution
//don't forget place it near executable 
char VertexShaderName[] = "Vertex.vert";
char FragmentShaderName[] = "Fragment.frag";

void initTexture()
{
	//generate as many textures as you need
	glGenTextures(1,&texId[0]);
	
	//enable texturing and zero slot
	glActiveTexture(GL_TEXTURE0);
	//bind texId to 0 unit
	glBindTexture(GL_TEXTURE_2D,texId[0]);
	
	//don't use alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	
	// Set nearest filtering mode for texture minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	GLubyte imgData[2*2*3] = {
		//row1: yellow,blue
		255,255,0, 0,0,255,
		//row2: green, red
		0,255,0, 255,0,0
	};  
	
	//set Texture Data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2,2, 0, GL_RGB, GL_UNSIGNED_BYTE, &imgData[0]);
}

/////////////////////////////////////////////////////////////////////
///is called when program starts
void init()
{
	//enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//initialize shader program
	shaderProgram.init(VertexShaderName,FragmentShaderName);
	//use this shader program
	glUseProgram(shaderProgram.programObject);
	
	land.Add(new LandSurface(0, 0, 0.16f));
	land.Add(new LandSurface(0, 1, 0.14f));
	land.Add(new LandSurface(1, 0, 0.1f));
	sea.Add(new SeaSurface());
	
	//initialize opengl buffers and variables inside of object
	land.InitGLBuffers(shaderProgram.programObject);
	sea.InitGLBuffers(shaderProgram.programObject);
	//initializa texture
	//initTexture();
}


/////////////////////////////////////////////////////////////////////
///called when window size is changed
void reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	//set viewport to match window size
	glViewport(0, 0, width, height);
	
	float fieldOfView = 45.0f;
	float aspectRatio = float(width)/float(height);
	float zNear = 0.1f;
	float zFar = 100.0f;
	//set projection matrix
	projectionMatrix = glm::perspective(fieldOfView,aspectRatio,zNear,zFar);
}

////////////////////////////////////////////////////////////////////
///camera moving control
void processCameraMoving()
{
	float amount = 0.001f;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int timeDiff = time - timePrev;
	timePrev = time;

	if (camMoveDir.z != 0)
	{
		glm::vec3 dist = eye - cen;
		glm::vec3 forward = glm::normalize(dist);		
		eye = eye + camMoveDir.z * amount * float(timeDiff) * forward;
		cen = cen + camMoveDir.z * amount * float(timeDiff) * forward;
	}
	if (camMoveDir.x != 0)
	{
		glm::vec3 dist = eye - cen;
		glm::vec3 right = glm::normalize(glm::cross(dist, up));
		eye = eye - camMoveDir.x * amount * float(timeDiff) * right;
		cen = cen - camMoveDir.x * amount * float(timeDiff) * right;
	}
	if (camMoveDir.y != 0)
	{
		eye = eye + camMoveDir.y * amount * float(timeDiff) * up;
		cen = cen + camMoveDir.y * amount * float(timeDiff) * up;
	}
}

////////////////////////////////////////////////////////////////////
///actions for single frame

bool SetShaderMatrix()
{
	//camera matrix. camera is placed in point "eye" and looks at point "cen".
	glm::mat4x4 viewMatrix = glm::lookAt(eye, cen, up);
	//glm::mat4x4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(-0.5f, -0.5f, 0));
	glm::mat4x4 modelMatrix = glm::mat4(1.0f);
	modelViewMatrix = viewMatrix * modelMatrix;	
	normalMatrix = glm::inverseTranspose(modelViewMatrix);
	modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;	
	
	//pass variables to the shaders
	
	int locMV = glGetUniformLocation(shaderProgram.programObject, "modelViewMatrix");
	if (locMV > -1)
		glUniformMatrix4fv(locMV, 1, 0, glm::value_ptr(modelViewMatrix));
	
	int locN = glGetUniformLocation(shaderProgram.programObject, "normalMatrix");
	if (locN > -1)
		glUniformMatrix4fv(locN, 1, 0, glm::value_ptr(normalMatrix));	  
	int locP = glGetUniformLocation(shaderProgram.programObject, "modelViewProjectionMatrix");
	if (locP > -1)
		glUniformMatrix4fv(locP, 1, 0, glm::value_ptr(modelViewProjectionMatrix));
		
	/*int texLoc = glGetUniformLocation(shaderProgram.programObject, "textureSampler");
	if (texLoc > -1)
		glUniform1ui(texLoc, 0);
	int locFlag = glGetUniformLocation(shaderProgram.programObject, "useTexture");
	if (locFlag > -1)
		glUniform1i(locFlag, useTexture);*/
  
	// light position
	glm::vec3 mvLightPos = glm::vec3(modelViewMatrix*glm::vec4(lightpos, 1));
	int locLight = glGetUniformLocation(shaderProgram.programObject, "lightPosition");
	if (locLight > -1)
		glUniform3fv(locLight, 1, glm::value_ptr(mvLightPos));

	return locMV > -1 && locN > -1 || locP > -1;
}

void display()
{
	//glClearColor(0, 0, 0, 0);
	//Sky Blue	135-206-250
	glClearColor(0.53f, 0.81f, 0.98f, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	//Draw objects with shaders (in screen coordinates)
	//need to set uniform in modelViewMatrix
	
	glUseProgram(shaderProgram.programObject);

	processCameraMoving();
	
	//bind texture
	glBindTexture(GL_TEXTURE_2D, texId[0]);
	bool matRes = SetShaderMatrix();	// draw main object
	if (matRes)
	{
		land.DrawReflection();
		land.Draw();
		sea.Draw();
	}
	else
	{
		//if there is some problem
		//not all uniforms were allocated - show blue screen.
		//check your variables properly. May be there is unused?
		glClearColor(0, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	//end frame visualization
	glutSwapBuffers();  
}

//////////////////////////////////////////////////////////////////////////
///IdleFunction
void update()
{
	//make animation
	glutPostRedisplay();
}

/////////////////////////////////////////////////////////////////////////
///is called when key on keyboard is pressed
void keyboard(unsigned char key, int mx, int my)
{
	if (key==' ')
		useTexture = !useTexture;

	switch (key)
	{
	case 'w':
		camMoveDir.z = -1;
		break;
	case 's':
		camMoveDir.z = 1;
		break;
	case 'd':
		camMoveDir.x = 1;
		break;
	case 'a':
		camMoveDir.x = -1;
		break;
	}
	glutPostRedisplay();
}

void specialKey(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_PAGE_UP: //up
          camMoveDir.y = 1;
          break;
	case GLUT_KEY_PAGE_DOWN: //down
          camMoveDir.y = -1;
          break;
	}
	glutPostRedisplay();
}

void specialKey_up(int key, int x, int y) {
	switch(key) {
	case GLUT_KEY_PAGE_UP: //up
          camMoveDir.y = 0;
          break;
	case GLUT_KEY_PAGE_DOWN: //down
          camMoveDir.y = 0;
          break;
	}
	glutPostRedisplay();
}

void keyboard_up(unsigned char key, int mx, int my)
{
	switch (key)
	{
	case 'w':
		camMoveDir.z = 0;
		break;
	case 's':
		camMoveDir.z = 0;
		break;
	case 'd':
		camMoveDir.x = 0;
		break;
	case 'a':
		camMoveDir.x = 0;
		break;
	}
	glutPostRedisplay();
}

/////////////////////////////////////////////////////////////////////////
///is called when mouse button is pressed
void mouse(int button, int mode,int posx, int posy)
{
	if (button==GLUT_LEFT_BUTTON)
	{
		if (mode == GLUT_DOWN)
		{
			mouseX = posx; mouseY = posy;
		}
		else
		{
			mouseX = -1; mouseY = -1;
		}		
	}
}

void mouseMove(int x, int y)
{	
	float amount = 0.08f;
	float deltaAngleX = (x - mouseX) * amount;
	float deltaAngleY = (y - mouseY) * amount;
	glm::vec4 forward = glm::vec4(cen - eye, 1.0);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), deltaAngleX, up);		// matrix for right and left rotation	
	forward = rotation * forward;											// forward vector right and left rotation
	glm::vec3 right = glm::normalize(glm::cross(glm::vec3(forward), up));
	rotation = glm::rotate(glm::mat4(1.0), deltaAngleY, right);				// matrix for up and down rotation
	forward = rotation * forward;											// forward vector up and down rotation
	cen = eye + glm::vec3(forward);
	mouseX = x; mouseY = y;
}

////////////////////////////////////////////////////////////////////////
///this function is used in case of InitializationError
void emptydisplay()
{
}

void InitRandomGenerator()
{
	srand((unsigned)time(0));
	int count = rand() % 5;
	for (int i = 0; i <= count; i++)
		rand();
}

////////////////////////////////////////////////////////////////////////
///entry point
int main (int argc, char* argv[])
{
	InitRandomGenerator();
	glutInit(&argc, argv);
#ifdef __APPLE__
	glutInitDisplayMode( GLUT_3_2_CORE_PROFILE| GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	//glutInitContextVersion (3, 2);
	glutInitContextVersion (3, 1);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
	glewExperimental = GL_TRUE;
#endif
	glutCreateWindow("Random Landscape");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutReshapeWindow(windowWidth,windowHeight);
	glutIdleFunc(update);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialFunc(specialKey);
	glutSpecialUpFunc(specialKey_up);
	glutMouseFunc(mouse);
	//glutPassiveMotionFunc(mouseMove);
	glutMotionFunc(mouseMove);
	timePrev = glutGet(GLUT_ELAPSED_TIME);

	glewInit();
	const char * slVer = (const char *) glGetString ( GL_SHADING_LANGUAGE_VERSION );
	cout << "GLSL Version: " << slVer << endl;

	try
	{
		init();
	}
	catch (const char *str)
	{
		cout << "Error During Initialiation: " << str << endl;
		glDeleteTextures(1,texId);
		//start main loop with empty screen
		glutDisplayFunc(emptydisplay);
		glutMainLoop();
		return -1;
	}

	try
	{
		glutMainLoop();
	}
	catch (const char *str)
	{
		cout << "Error During Main Loop: " << str << endl;
	}
	//release memory
	//
	glDeleteTextures(1,texId);
	return 0;
}
