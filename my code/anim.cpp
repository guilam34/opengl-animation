// anim.cpp version 5.0 -- Template code for drawing an articulated figure.  CS 174A.

#ifdef _WIN32
#pragma comment(lib, "../GL/Win32/freeglut.lib")
#pragma comment(lib, "../GL/Win32/glew32s.lib")
#pragma comment(lib, "../GL/Win32/glew32mxs.lib")
#else
#pragma comment(lib, "../GL/x64/freeglut.lib")
#pragma comment(lib, "../GL/x64/glew32s.lib")
#pragma comment(lib, "../GL/x64/glew32mxs.lib")
#endif

#define _CRT_SECURE_NO_DEPRECATE
#ifndef EMSCRIPTEN
#include <Windows.h>
#define GLEW_STATIC
#include "..\GL\glew.h"
#endif
#define GL_GLEXT_PROTOTYPES
#include "..\GL\freeglut.h"

#include <math.h>
#include <assert.h>
#include "../CS174A template/tga.h"
#include <cmath>
#include <stack>
#include <ctime>

#include "../CS174a template/ArcBall.h"
#include "../CS174a template/FrameSaver.h"
#include "../CS174a template/Timer.h"
#include "../CS174a template/Shapes.h"
#include "../CS174a template/mat.h"
#include "../CS174a template/vec.h"
#include "../CS174a template/InitShaders.h"

#ifdef __APPLE__
#define glutInitContextVersion(a,b)
#define glutInitContextProfile(a)
#define glewExperimental int glewExperimentalAPPLE
#define glewInit()
#endif

FrameSaver FrSaver ;
Timer TM ;

BallData *Arcball = NULL ;
int Width = 800, Height = 800 ;
float Zoom = 1 ;

int Animate = 1, Recording = 0 ;

const int STRLEN = 100;
typedef char STR[STRLEN];

#define X 0
#define Y 1
#define Z 2

/*-------------------------------
---------------------------------
---------------------------------*/

#define DOGEINTROSPIN 0
#define DOGEINTROSTAND 0.5
#define DOGEINTROTOPSPIN 0.75
#define DOGEWALKING 1
#define DOGEWADDLING 1.5
#define DOGEWALLSTOP 1.75
#define DOGEBLOCKLOCK 2
#define DOGEWALLOPEN 2.5
#define DOGECROSSWALL 2.75
#define DOGELOCKHOP 3
#define DOGEGETBONE 4

float jumpBeginTime = 0.5;
float timeToJump = 4.5;
float jumpSceneTime; //start time: 0.5, end time:5
float walkBeginTime=5;
float timeToWalk = 6;
float walkSceneTime; //start time: 5, end time:11
float waddleBeginTime = 11;
float timeToWaddle=4;	
float waddleSceneTime;
float stopBeginTime = 14;
float timeToStop = 2.5;
float stopSceneTime;
float unlockBeginTime = 16.5;
float timeToUnlock = 8;
float unlockSceneTime;
float crossBeginTime = 24.5;
float timeToCross = 5.5;
float crossSceneTime;
float hopBeginTime = 30;
float timeToHop =8;
float hopSceneTime;
float boneBeginTime = 38;
float timeToBone = 5.5;
float boneSceneTime;
/*--------------------------------
----------------------------------
----------------------------------*/

GLuint texture_cube, texture_grass, texture_wall, texture_top, texture_lock;

// Structs that hold the Vertex Array Object index and number of vertices of each shape.
ShapeData cubeData, sphereData, coneData, cylData, topData;
unsigned int frame_buffer = 0;
mat4         model_view;
GLint        uModelView, uProjection, uView,
			 uAmbient, uDiffuse, uSpecular, uLightPos, uShininess,
			 uTex, uEnableTex;

// The eye point and look-at point.
// Currently unused. Use to control a camera with LookAt().
Angel::vec4 eye(0, 0.0, 20.0,1.0);
Angel::vec4 ref(0.0, 0.0, 0.0,1.0);
Angel::vec4 up(0.0,1.0,0.0,0.0);
Angel::vec4 circleDoge(-10.0,5.0,0.0,1.0);
Angel::vec4 waddleDoge(5.0,0,0,1.0);
double TIME = 0.0 ;
std::stack<mat4> mvstack;

void instructions() 
{
    printf("Press:\n");
    printf("  s to save the image\n");
    printf("  r to restore the original view.\n") ;
    printf("  0 to set it to the zero state.\n") ;
    printf("  a to toggle the animation.\n") ;
    printf("  m to toggle frame dumping.\n") ;
    printf("  q to quit.\n");
}

void drawCylinder(void)	//render a solid cylinder oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(model_view));
	glBindVertexArray(cylData.vao);
	glDrawArrays(GL_TRIANGLES, 0, cylData.numVertices);
}

void drawCone(void)	//render a solid cone oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(model_view));
	glBindVertexArray(coneData.vao);
	glDrawArrays(GL_TRIANGLES, 0, coneData.numVertices);
}

void drawTop(void)	//render a solid cone oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(model_view));
	glBindVertexArray(topData.vao);
	glDrawArrays(GL_TRIANGLES, 0, topData.numVertices);
}

void drawDogeTop()
{
	glBindTexture(GL_TEXTURE_2D, texture_top);
	glUniform1i(uEnableTex, 1);
	glUniformMatrix4fv(uModelView, 1, GL_TRUE, model_view);
	glBindVertexArray(topData.vao);
	glDrawArrays(GL_TRIANGLES, 0, topData.numVertices);
	glUniform1i(uEnableTex, 0);
}

void drawCube(void)		// draw a cube with dimensions 1,1,1 centered around the origin.
{
	glBindTexture(GL_TEXTURE_2D, texture_cube);
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(model_view));
	glBindVertexArray(cubeData.vao);
	glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
}

void drawLockPart(void)
{
	glBindTexture(GL_TEXTURE_2D, texture_lock);
	glUniform1i(uEnableTex, 1);
	glUniformMatrix4fv(uModelView, 1, GL_TRUE, model_view);
	glBindVertexArray(cubeData.vao);
	glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
	glUniform1i(uEnableTex, 0);
}

void drawGround(void)
{
	glBindTexture(GL_TEXTURE_2D, texture_grass);
	glUniform1i(uEnableTex, 1);
	glUniformMatrix4fv(uModelView, 1, GL_TRUE, model_view);
	glBindVertexArray(cubeData.vao);
	glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
	glUniform1i(uEnableTex, 0);
}

void drawWall(GLuint texture_var)
{
	glBindTexture(GL_TEXTURE_2D, texture_var);
	glUniform1i(uEnableTex, 1);
	glUniformMatrix4fv(uModelView, 1, GL_TRUE, model_view);
	glBindVertexArray(cubeData.vao);
	glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
	glUniform1i(uEnableTex, 0);
}

void drawSphere(void)	// draw a sphere with radius 1 centered around the origin.
{	
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(model_view));
	glBindVertexArray(sphereData.vao);
	glDrawArrays(GL_TRIANGLES, 0, sphereData.numVertices);
}

// this function gets caled for any keypresses
void myKey(unsigned char key, int x, int y)
{
    float time ;
    switch (key) {
        case 'q':
        case 27:
            exit(0); 
        case 's':
            FrSaver.DumpPPM(Width,Height) ;
            break;
        case 'r':
			Ball_Init(Arcball);					// reset arcball
			Ball_Place(Arcball,qOne,0.75);
            glutPostRedisplay() ;
            break ;
        case 'a': // toggle animation
            Animate = 1 - Animate ;
            // reset the timer to point to the current time		
            time = TM.GetElapsedTime() ;
            TM.Reset() ;
            // printf("Elapsed time %f\n", time) ;
            break ;
        case '0':
            //reset your object
            break ;
        case 'm':
            if( Recording == 1 )
            {
                printf("Frame recording disabled.\n") ;
                Recording = 0 ;
            }
            else
            {
                printf("Frame recording enabled.\n") ;
                Recording = 1  ;
            }
            FrSaver.Toggle(Width);
            break ;
        case 'h':
        case '?':
            instructions();
            break;
    }
    glutPostRedisplay() ;
}

// Performs most of the OpenGL intialization -- change these with care, if you must.
void myinit(void)		
{

#ifndef EMSCRIPTEN
    GLuint program = InitShader( "../my code/vshader.glsl", "../my code/fshader.glsl" );		// Load shaders and use the resulting shader program
#else
	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );								// Load shaders and use the resulting shader program
#endif
    glUseProgram(program);		

    // Generate vertex arrays for geometric shapes
    generateCube(program, &cubeData);
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);
	generateTop(program, &topData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView       = glGetUniformLocation( program, "View"       );

    glClearColor( 0.1, 0.1, 0.2, 1.0 ); // dark blue background

    uAmbient   = glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse   = glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular  = glGetUniformLocation( program, "SpecularProduct" );
    uLightPos  = glGetUniformLocation( program, "LightPosition"   );
    uShininess = glGetUniformLocation( program, "Shininess"       );
    uTex       = glGetUniformLocation( program, "Tex"             );
    uEnableTex = glGetUniformLocation( program, "EnableTex"       );

    glUniform4f(uAmbient,    0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uDiffuse,    0.6f,  0.6f,  0.6f, 1.0f);
    glUniform4f(uSpecular,   0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uLightPos,  15.0f, 15.0f, 30.0f, 0.0f);
    glUniform1f(uShininess, 100.0f);

    glEnable(GL_DEPTH_TEST);
 
	TgaImage wall_image;
	if (!wall_image.loadTGA("Textures/wall_512_5_05.tga"))
	{
		printf("Error loading image file\n");
		exit(1);
	}
	TgaImage grass_image;
	if (!grass_image.loadTGA("Textures/grass.tga"))
	{
		printf("Error loading image file\n");
		exit(1);
	}
	TgaImage top_image;
	if (!top_image.loadTGA("Textures/top.tga"))
	{
		printf("Error loading image file\n");
		exit(1);
	}
	TgaImage lock_image;
	if (!lock_image.loadTGA("Textures/lock.tga"))
	{
		printf("Error loading image file\n");
		exit(1);
	}
	glGenTextures(1, &texture_wall);
	glBindTexture(GL_TEXTURE_2D, texture_wall);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, wall_image.width, wall_image.height, 0,
		(wall_image.byteCount == 3) ? GL_BGR : GL_BGRA,
		GL_UNSIGNED_BYTE, wall_image.data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &texture_grass);
	glBindTexture(GL_TEXTURE_2D, texture_grass);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, grass_image.width, grass_image.height, 0,
		(grass_image.byteCount == 3) ? GL_BGR : GL_BGRA,
		GL_UNSIGNED_BYTE, grass_image.data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &texture_top);
	glBindTexture(GL_TEXTURE_2D, texture_top);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, top_image.width, top_image.height, 0,
		(top_image.byteCount == 3) ? GL_BGR : GL_BGRA,
		GL_UNSIGNED_BYTE, top_image.data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &texture_lock);
	glBindTexture(GL_TEXTURE_2D, texture_lock);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, lock_image.width, lock_image.height, 0,
		(lock_image.byteCount == 3) ? GL_BGR : GL_BGRA,
		GL_UNSIGNED_BYTE, lock_image.data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glUniform1i(uTex, 0);

    Arcball = new BallData;
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}

void set_colour(float r, float g, float b)		// sets all material properties to the given colour
{
    float ambient  = 0.2f, diffuse  = 0.6f, specular = 0.2f;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1.0f);
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1.0f);
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1.0f);
}

/*********************************************************
**********************************************************
**********************************************************/

void genWall(float rotato)
{
		mvstack.push(model_view);
		if (rotato == DOGEWADDLING)
		{
			model_view *= Translate(-15, 0, 0);
		}
		else if (rotato == DOGEWALLSTOP)
		{
			model_view *= Translate(-10, 0, 0);
		}
		else if (rotato == DOGEBLOCKLOCK || rotato==DOGEWALLOPEN)
		{
			model_view *= Translate(0, 0, -10);
			model_view *= RotateY(105);
		}
		else if (rotato == DOGEGETBONE)
		{
			model_view *= Translate(0,0,-80);
			model_view *= RotateY(90);
		}
		if (rotato == DOGEGETBONE)
		{
			model_view *= Scale(1.5, 40, 50);
		}			
		else{
			model_view *= Scale(1.5, 20, 15);
		}
		model_view *= Translate(0, 0, 0.5);
		if (rotato == DOGEWALLOPEN)
		{
			float rel_time = (unlockSceneTime - 3) / (timeToUnlock - 3);
			if (unlockSceneTime < timeToUnlock - 2)
			{
				model_view *= Translate(0, 0, rel_time*0.6);
			}
			else{
				model_view *= Translate(0, 0, 0.36);
			}
		}
		if (rotato == DOGECROSSWALL)
		{
			model_view *= Translate(0, 0, 0.36);
		}
		if (rotato==DOGEGETBONE)
		{
			model_view *= Translate(0, 0, 0.2);
		}
		drawWall(texture_wall);
		model_view = mvstack.top();
		if (rotato == DOGEWADDLING)
		{
			model_view *= Translate(-15, 0, 0);
		}
		else if (rotato == DOGEWALLSTOP)
		{
			model_view *= Translate(-10, 0, 0);
		}
		else if (rotato == DOGEBLOCKLOCK || rotato == DOGEWALLOPEN)
		{
			model_view *= Translate(0, 0, -10);
			model_view *= RotateY(105);
		}
		else if (rotato == DOGEGETBONE)
		{
			model_view *= Translate(0, 0, -80);
			model_view *= RotateY(90);
		}
		if (rotato == DOGEGETBONE)
		{
			model_view *= Scale(1.5, 40,50);
		}
		else{
			model_view *= Scale(1.5, 20, 15);
		}
		model_view *= Translate(0, 0, -0.5);
		if (rotato == DOGEWALLOPEN)
		{
			if (unlockSceneTime < timeToUnlock - 2)
			{
				float rel_time = (unlockSceneTime - 3) / (timeToUnlock - 3);
				model_view *= Translate(0, 0, -rel_time*0.6);
			}
			else{
				model_view *= Translate(0, 0, -0.36);
			}
		}
		if (rotato == DOGECROSSWALL)
		{
			model_view *= Translate(0, 0, -0.36);
		}
		if (rotato == DOGEGETBONE)
		{
			model_view *= Translate(0, 0, -0.2);
		}
		drawWall(texture_wall);
		model_view = mvstack.top();
		mvstack.pop();

}
 
void drawRoad(float rotato)
{
	model_view = mvstack.top();
	set_colour(0.2,0.2,0.2);
	model_view *= Translate(0, -1.99, 0);
	model_view *= Scale(400, 1, 10);
	model_view *= Translate(0,0,-4);
	drawCube();
	model_view *= Scale(0.0025, 1, 0.1);
	model_view *= Scale(0.2,1,0.2);
	model_view *= RotateX(90);
	model_view *= Translate(0,0,-5);
	mvstack.push(model_view);
	set_colour(0.4,0.2,0);
	model_view = mvstack.top();
	for (int x = -750; x < 750; x=x+15)
	{
		model_view = mvstack.top();
		model_view *= Translate(x, 30, 3.5);
		drawCylinder();
	}
	model_view = mvstack.top();
	model_view *= Scale(1500, 1, 0.5);
	model_view *= Translate(0, 30, 6);
	model_view *= Scale(1, 1, 0.8);
	drawCube();
	model_view *= Translate(0, 0, 2);
	drawCube();
	for (int x = -750; x < 750; x = x + 15)
	{
		model_view = mvstack.top();
		model_view *= Translate(x+5, -30, 3.5);
		drawCylinder();
	}
	model_view = mvstack.top();
	model_view *= Scale(1500, 1, 0.5);
	model_view *= Translate(0, -30, 6);
	model_view *= Scale(1, 1, 0.8);
	drawCube();
	model_view *= Translate(0, 0, 2);
	drawCube();
	mvstack.pop();
	if (rotato <=DOGEWALKING)
	{
		float rel_time = jumpSceneTime;
		model_view = mvstack.top();
		if (rotato <= DOGEINTROTOPSPIN)
		{
			model_view *= Translate(80 - 20*rel_time, 0, 0);
		}
		else if (rotato == DOGEWALKING)
		{
			model_view *= Translate(120 - 20 * rel_time, 0, 0);
		}
		model_view *= Translate(0,0,-40);
		set_colour(0.3, 0.3, 0.3);
		model_view *= Scale(6,3,3.5);
		model_view *= Translate(0,0.5,0);
		drawCube();
		model_view *= Translate(0,-0.3,0);
		model_view *= Scale(1.25,1,0.99);
		model_view *= Translate(-0.125,0 , 0);
		set_colour(0,0,0.2);
		drawCube();
		set_colour(0.125,0.125,0.125);
		model_view = mvstack.top();
		if (rotato <= DOGEINTROTOPSPIN)
		{
			model_view *= Translate(80 - 20 * rel_time, 0, 0);
		}
		else if (rotato == DOGEWALKING)
		{
			model_view *= Translate(120 - 20 * rel_time, 0, 0);
		}
		model_view *= Translate(0, 0, -40);
		model_view *= Scale(0.7, 0.7, 0.3);
		model_view *= Translate(-4.25, -1.2, 5);
		drawCylinder();
		model_view = mvstack.top();
		if (rotato <= DOGEINTROTOPSPIN)
		{
			model_view *= Translate(80 - 20 * rel_time, 0, 0);
		}
		else if (rotato == DOGEWALKING)
		{
			model_view *= Translate(120 - 20 * rel_time, 0, 0);
		}
		model_view *= Translate(0, 0, -40);
		model_view *= Scale(0.7, 0.7, 0.3);
		model_view *= Translate(-4.25, -1.2, -5);
		drawCylinder();
		model_view = mvstack.top();
		if (rotato <= DOGEINTROTOPSPIN)
		{
			model_view *= Translate(80 - 20 * rel_time, 0, 0);
		}
		else if (rotato == DOGEWALKING)
		{
			model_view *= Translate(120 - 20 * rel_time, 0, 0);
		}
		model_view *= Translate(0, 0, -40);
		model_view *= Scale(0.7, 0.7, 0.3);
		model_view *= Translate(2, -1.2, 5);
		drawCylinder();
		model_view = mvstack.top();
		if (rotato <= DOGEINTROTOPSPIN)
		{
			model_view *= Translate(80 - 20 * rel_time, 0, 0);
		}
		else if (rotato == DOGEWALKING)
		{
			model_view *= Translate(120 - 20 * rel_time, 0, 0);
		}
		model_view *= Translate(0, 0, -40);
		model_view *= Scale(0.7, 0.7, 0.3);
		model_view *= Translate(2, -1.2, -5);
		drawCylinder();
		float start_time = rel_time - 1;
		if (rotato == DOGEWALKING && start_time>0)
		{			
			model_view = mvstack.top();
			model_view *= Translate(-115 + 18 * start_time, 0, 0);
			model_view *= Translate(0,0,-40);
			set_colour(0.6, 0, 0);
			model_view *= Scale(2, 3, 3.5);
			model_view *= Translate(0, 0.3, 0);
			drawCube();
			model_view *= Scale(2.5, 0.5, 0.99);
			model_view *= Translate(-0.5,-0.5,0);
			drawCube();
			model_view *= Scale(0.4,1.2,1);
			model_view *= Translate(2.2,0.075,0);
			drawCube();
			set_colour(0.125, 0.125, 0.125);
			model_view = mvstack.top();
			model_view *= Translate(-115 + 18 * start_time, 0, 0);
			model_view *= Translate(0, 0, -40);
			model_view *= Scale(0.8, 0.8, 0.35);
			model_view *= Translate(-4, -0.8, 5);
			drawCylinder();
			model_view = mvstack.top();
			model_view *= Translate(-115 + 18 * start_time, 0, 0);
			model_view *= Translate(0, 0, -40);
			model_view *= Scale(0.8, 0.8, 0.35);
			model_view *= Translate(-4, -0.8, -5);
			drawCylinder();
			model_view = mvstack.top();
			model_view *= Translate(-115 + 18 * start_time, 0, 0);
			model_view *= Translate(0, 0, -40);
			model_view *= Scale(0.8, 0.8, 0.35);
			model_view *= Translate(1.75, -0.8, 5);
			drawCylinder();
			model_view = mvstack.top();
			model_view *= Translate(-115 + 18 * start_time, 0, 0);
			model_view *= Translate(0, 0, -40);
			model_view *= Scale(0.8, 0.8, 0.35);
			model_view *= Translate(1.75, -0.8, -5);
			drawCylinder();
		}
	}
	if (rotato == DOGELOCKHOP)
	{
		set_colour(0.2, 0.1, 0);
		float rel_time = hopSceneTime;
		for (int x = 0; x < 8; x++)
		{
			model_view = mvstack.top();
			model_view *= Translate(0,0,-25);
			model_view *= RotateX(-90);
			model_view *= Translate(0,0,-1);			
			model_view *= Translate(0,0,0.5+x);
			drawCylinder();
		}
		set_colour(0,0.2,0);
		model_view *= Translate(0,0,6);
		model_view *= Scale(8,8,5);
		drawSphere();
		set_colour(0.2, 0.1, 0);
		for (int x = 0; x < 8; x++)
		{
			model_view = mvstack.top();
			model_view *= Translate(-10, 0, -17.5); //-10,0,-17.5
			model_view *= RotateX(-90);
			model_view *= Translate(0, 0, -1);
			model_view *= Translate(0, 0, 0.5 + x);
			drawCylinder();
		}
		set_colour(0, 0.2, 0);
		model_view *= Translate(0, 0, 2);
		model_view *= Scale(7, 7, 4.5);
		drawSphere();
		set_colour(0.2, 0.1, 0);
		for (int x = 0; x < 8; x++)
		{
			model_view = mvstack.top();
			model_view *= Translate(7.5, 0, -10);
			model_view *= RotateX(-90);
			model_view *= Translate(0, 0, -1);
			model_view *= Translate(0, 0, 0.5 + x);
			drawCylinder();
		}
		set_colour(0, 0.2, 0);
		model_view *= Translate(0, 0, 4);
		model_view *= Scale(7, 7, 4.5);
		drawSphere();


	}

	set_colour(1,1,1);
}

void genGround(float rotato)
{
		mvstack.push(model_view);
		set_colour(0.247, 0.455, 0.169);
		model_view *= Translate(0, -2, 0);
		model_view *= Scale(500, 1, 500);
		drawGround();
		if (rotato<DOGEBLOCKLOCK || (rotato>=DOGELOCKHOP && rotato<DOGEGETBONE))
			drawRoad(rotato);
		set_colour(1, 1, 1);
		model_view = mvstack.top();
		mvstack.pop();
}

void genBone(void)
{	
	float rel_time = (boneSceneTime - 3.75)/0.75;
	mvstack.push(model_view);
	model_view *= RotateY(90);
	model_view *= RotateX(-90);	
	model_view *= Translate(0, 0, -2);	
	set_colour(0, 0.285, 0.6);
	drawCylinder();
	set_colour(1, 1, 1);
	model_view = mvstack.top();
	model_view *= RotateY(90);
	model_view *= Scale(0.3, 0.3, 1);
	model_view *= Translate(0, -2.5, 0);
	if (boneSceneTime >= 3.75 && boneSceneTime < 4.5)
	{
		model_view *= Translate(1*rel_time, 11*rel_time, 0);
	}
	else if (boneSceneTime >= 4.5)
	{
		model_view *= Translate(1,11,0);
	}
	drawCylinder();
	model_view = mvstack.top();
	model_view *= RotateY(90);
	model_view *= Scale(0.4, 0.4, 0.4);
	model_view *= Translate(-0.5, -2, 2.5);
	if (boneSceneTime >= 3.75 && boneSceneTime < 4.5)
	{
		model_view *= Translate(1 * rel_time, 8.5 * rel_time, 0);
	}
	else if (boneSceneTime >= 4.5)
	{
		model_view *= Translate(1,8.5,0);
	}
	drawSphere();
	model_view = mvstack.top();
	model_view *= RotateY(90);
	model_view *= Scale(0.4, 0.4, 0.4);
	model_view *= Translate(-0.5, -2, -2.5);
	if (boneSceneTime >= 3.75 && boneSceneTime < 4.5)
	{
		model_view *= Translate(1 * rel_time, 8.5 * rel_time, 0);
	}
	else if (boneSceneTime >= 4.5)
	{
		model_view *= Translate(1, 8.5, 0);
	}
	drawSphere();
	model_view = mvstack.top();
	model_view *= RotateY(90);
	model_view *= Scale(0.4, 0.4, 0.4);
	model_view *= Translate(0.5, -2, 2.5);
	if (boneSceneTime >= 3.75 && boneSceneTime < 4.5)
	{
		model_view *= Translate(1 * rel_time, 8.5 * rel_time, 0);
	}
	else if (boneSceneTime >= 4.5)
	{
		model_view *= Translate(1, 8.5, 0);
	}
	drawSphere();
	model_view = mvstack.top();
	model_view *= RotateY(90);
	model_view *= Scale(0.4, 0.4, 0.4);
	model_view *= Translate(0.5, -2, -2.5);
	if (boneSceneTime >= 3.75 && boneSceneTime < 4.5)
	{
		model_view *= Translate(1 * rel_time, 8.5 * rel_time, 0);
	}
	else if (boneSceneTime >= 4.5)
	{
		model_view *= Translate(1, 8.5, 0);
	}
	drawSphere();
	model_view = mvstack.top();
	mvstack.pop();
}

void genLock(int rotato)
{
	if (rotato == DOGELOCKHOP)
	{
		model_view *= Translate(0,0,-5);
	}
	if (rotato == DOGEGETBONE)
	{
		model_view *= Translate(0, 0, -40);
	}
	mvstack.push(model_view);
	set_colour(0.4275,0.055,0.055);	
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(0, -1, 6);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(0, -1, 4);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(1, -1, 5);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(1, -1, 4);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(1, -1, 6);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(-1, -1, 5);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(-1, -1, 4);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.4275, 0.055, 0.055);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(-1, -1, 6);
	drawLockPart();
	model_view = mvstack.top();
	set_colour(0.8, 0.8, 0.8);
	if (rotato == DOGEBLOCKLOCK)
	{
		model_view *= RotateY(15);
	}
	model_view *= Translate(0, 0, 5);
	if (unlockSceneTime < 2 && rotato==DOGEBLOCKLOCK)
	{
		float rel_time = (unlockSceneTime) / (timeToUnlock - 6);
		model_view *= Translate(0, 15 * sin(rel_time*DegreesToRadians * 180), -15.5 * cos(-rel_time*DegreesToRadians * 180) - 15);
	}
	else
		model_view *= Translate(0,-0.5,0);
	set_colour(0,0.5,1);
	drawTop();
	set_colour(1,1,1);
	model_view = mvstack.top();
	mvstack.pop();
}

void genDogeTorso(float rotato)
{
	mvstack.push(model_view);			//Torso	
	if (rotato >DOGEINTROTOPSPIN)
	{
		if (rotato == DOGEWALKING)
		{
			float rel_time = walkSceneTime - timeToWalk;
			model_view *= Translate((rel_time * -3) - 7, 0, 0);
			model_view *= RotateZ(1.5 * sin(rel_time * 10));
		}
		if (rotato == DOGEWADDLING)
		{
			float rel_time = waddleSceneTime - timeToWaddle;
			model_view *= Translate((rel_time * -3) - 7, 0, 0);
			model_view *= RotateZ(1.5 * sin(rel_time * 10));
		}
		if (rotato == DOGEWALLOPEN)
		{
			float rel_time = (unlockSceneTime - 3) / (timeToUnlock - 3);
			model_view *= Translate((rel_time * -8), 0, 0);
			model_view *= RotateZ(1.5 * sin(rel_time * 10));
		}
		if (rotato == DOGECROSSWALL)
		{
			float rel_time = crossSceneTime / timeToCross;
			model_view *= Translate((rel_time * -8), 0, 0);
			model_view *= RotateZ(1.5 * sin(rel_time * 10));
		}
		if (rotato == DOGELOCKHOP)
		{
			float rel_time;
			if (hopSceneTime < 3)
			{				
				model_view *= Translate((hopSceneTime*-5*0.333), 0, 0);
				model_view *= RotateZ(1.5 * sin(hopSceneTime*0.33 * 10));
			}
			else if (hopSceneTime >= 3 && hopSceneTime<4)
			{
				rel_time = hopSceneTime - 3;
				model_view *= Translate((3 * -5 * 0.333), 0, 0);
				model_view *= Translate(4 * cos(180 *rel_time*DegreesToRadians)-4, 2 * sin(180 *rel_time*DegreesToRadians), 0);
				model_view *= RotateZ(-15*sin(360*rel_time*DegreesToRadians));
			}
			else if (hopSceneTime>=4)
			{
				rel_time = hopSceneTime - 4;
				model_view *= Translate((3*-5*0.333)-8,0,0);
				model_view *= Translate((rel_time*-5 * 0.333), 0, 0);
				model_view *= RotateZ(1.5 * sin(hopSceneTime*0.33 * 10));
			}
		}
		if (rotato == DOGEGETBONE)
		{
			float rel_time;
			if (boneSceneTime < 3)
			{
				rel_time = boneSceneTime;
				model_view *= Translate((rel_time * -2.4), 0, 0);
				model_view *= RotateZ(1.5 * sin(rel_time * 10));
			}
			else{
				model_view *= Translate(3*-2.4,0,0);
			}
		}
	}
	model_view *= RotateY(90);
	model_view *= RotateX(10);
	model_view *= Scale(0.7, 0.7, 1);
	drawCylinder();
}

void genDogeHead(float rotato)
{
	model_view = mvstack.top();	
	model_view *= RotateX(40);
	if (rotato == DOGEGETBONE)
	{
		float rel_time = boneSceneTime - 3;
		if (rel_time >= 0 && rel_time<1.5)
		{
			model_view *= RotateX(-50 * sin(rel_time*2*0.333 * 180 * DegreesToRadians));
		}
	}
	model_view *= Scale(0.4, 0.4, 1.25);
	drawCylinder(); //draw neck
	model_view = mvstack.top();
	set_colour(1.0f, 0.0f, 0.0f);
	model_view *= RotateX(40);
	if (rotato == DOGEGETBONE)
	{
		float rel_time = boneSceneTime - 3;
		if (rel_time >= 0 && rel_time<1.5)
		{
			model_view *= RotateX(-50 * sin(rel_time * 2 * 0.333 * 180 * DegreesToRadians));
		}
	}
	model_view *= Scale(0.5, 0.5, 1.1);
	drawCylinder(); //draw collar
	set_colour(1, 1, 1);
	if (rotato == DOGEWALLSTOP)
	{
		if (stopSceneTime > 0.75 && stopSceneTime<1.25)
		{
			float rel_time = (stopSceneTime - timeToStop + 1.75)/0.5;
			model_view *= RotateX(6 * sin(rel_time*DegreesToRadians * 90));
		}
	}
	if (rotato == DOGEINTROTOPSPIN)
	{
		float rel_time = jumpSceneTime - timeToJump + 1;
		model_view *= RotateX(4*sin(rel_time*DegreesToRadians*720));
	}
	
	model_view *= Scale(2, 2, 1);		
	mvstack.push(model_view); //return to original 1:1:1 ratio
	model_view *= Translate(0, 0, -1.6);
	model_view *= Scale(0.8, 0.8, 0.8);
	if (rotato>=DOGEWALLOPEN)
		model_view *= RotateX(-20);	
	drawSphere(); //draw head

	mvstack.push(model_view); //center at the head, 1:1:1 ratio	
	model_view *= RotateX(-40);
	model_view *= RotateX(90);
	model_view *= RotateY(-30);
	model_view *= RotateX(20);
	model_view *= Translate(0,0,-1);
	model_view *= Scale(0.6, 0.6, 0.6);
	drawCone();

	model_view = mvstack.top();
	model_view *= RotateX(-40);
	model_view *= RotateX(90);
	model_view *= RotateY(30);
	model_view *= RotateX(20);
	model_view *= Translate(0, 0, -1);
	model_view *= Scale(0.6, 0.6, 0.6);
	drawCone();

	model_view = mvstack.top();
	model_view *= RotateX(-20);
	model_view *= Translate(0, 0, -0.7);
	model_view *= Scale(0.45, 0.45, 0.7);
	drawCylinder();	//draw nose

	mvstack.push(model_view); //push snout position
	model_view = mvstack.top();
	set_colour(0.8, 0.8, 0.8);
	model_view *= Translate(0, 0, -1);
	model_view *= Translate(0, 0.5, 0);
	model_view *= Scale(2.22, 2.22, 1.43);
	if (rotato > DOGEINTROSPIN)
	{
		if (rotato == DOGEINTROTOPSPIN)
		{
			float rel_time = jumpSceneTime - timeToJump + 1;
			model_view *= Translate(0, 0.25*sin(rel_time*DegreesToRadians * 720 + 270) + 0.25, 0);
		}
		else if (rotato == DOGEWALLOPEN)
		{
			if (unlockSceneTime >= 3)
			{
				float rel_time = (unlockSceneTime-3)/6*5;
				model_view *= Translate(0, 0.25*sin(rel_time*DegreesToRadians * 720 + 270) + 0.25, 0);
			}
		}
		else{
			if (rotato == DOGEWALLSTOP)
			{
				if (stopSceneTime < 1.25)
					model_view *= RotateX(5 * sin(TIME * 5));
			}
			else
				model_view *= RotateX(5 * sin(TIME * 5));
		}
	}
	model_view *= Translate(0, 1, 0);
	model_view *= RotateX(-20);	
	
	if (rotato>DOGEINTROSPIN && rotato!=DOGEWALLSTOP)
		model_view *= RotateY(TIME * 500);
	if (rotato == DOGEWALLSTOP)
	{
		if (stopSceneTime >= 1.25 && stopSceneTime<2.5)
		{
			float rel_time = (stopSceneTime - timeToStop + 1.25)/1.25;
			model_view *= Translate(0, 6 * sin(rel_time*DegreesToRadians * 360 + 270) + 6, -40 * rel_time);
		}
	}
	if (rotato >DOGEINTROSPIN&&rotato < DOGEWALLOPEN)
	{
		set_colour(0,0.5,1);
		drawTop();
	} //DRAWTOP

	model_view = mvstack.top();	
	set_colour(1, 1, 1);
	model_view *= Translate(0, 0, -0.5);
	model_view *= Scale(1.15, 1.15, 1);
	drawSphere();	//draw round part of snout

	model_view = mvstack.top();
	mvstack.pop();
	model_view *= Translate(0, 0, -1.5);
	model_view *= Scale(0.2, 0.2, 0.2);
	set_colour(0, 0, 0);
	drawSphere();	//draw nose
	set_colour(1, 1, 1);
	mvstack.pop(); //pop head center
	mvstack.pop(); //pop resizing to 1:1:1
}

void genDogeFront(float rotato)
{
	mvstack.push(model_view);			//Front legs
	model_view *= Scale(1.02, 1.02, 0.7);
	model_view *= Translate(0, 0, -1.2);
	drawSphere();
	model_view *= RotateX(-10);
	mvstack.push(model_view);
	model_view *= Translate(0.5, -0.3, 0);
	model_view *= RotateX(-90);
	if (rotato >DOGEINTROTOPSPIN && rotato<DOGEWALLSTOP)
	{
		float rel_time = walkSceneTime - timeToWalk;
		model_view *= RotateX(10 * sin(rel_time * 10));
	}
	else if (rotato == DOGEWALLOPEN)
	{
			float rel_time = (unlockSceneTime - 3);
			model_view *= RotateX(10 * sin(rel_time * 10));
	}
	else if (rotato == DOGECROSSWALL)
	{
		float rel_time = crossSceneTime;
		model_view *= RotateX(10 * sin(rel_time * 10));
	}
	else if (rotato == DOGELOCKHOP)
	{
		float rel_time;
		if (hopSceneTime < 3 || hopSceneTime >= 4)
		{
			rel_time = hopSceneTime;
			model_view *= RotateX(10 * sin(rel_time * 10));
		}
		else if (hopSceneTime >= 3 && hopSceneTime < 4)
		{
			rel_time = hopSceneTime - 3;
			model_view *= RotateX(30*sin((360*rel_time-90)*DegreesToRadians));
			model_view *= RotateX(30);
		}
	}
	else if (rotato == DOGEGETBONE)
	{
		float rel_time;
		if (boneSceneTime < 3)
		{
			rel_time = boneSceneTime;
			model_view *= RotateX(10 * sin(rel_time * 10));
		}
	}
	model_view *= Scale(0.3, 0.3, 1);
	model_view *= Translate(0, 0, -1);
	drawCylinder();
	model_view = mvstack.top();
	model_view *= Translate(-0.5, -0.3, 0);
	model_view *= RotateX(-90);
	if (rotato >DOGEINTROTOPSPIN && rotato<DOGEWALLSTOP)
	{
		float rel_time = walkSceneTime - timeToWalk;
		model_view *= RotateX(-10 * sin(rel_time * 10));
	}
	else if (rotato == DOGEWALLOPEN)
	{
			float rel_time = (unlockSceneTime - 3);
			model_view *= RotateX(-10 * sin(rel_time * 10));
	}
	else if (rotato == DOGECROSSWALL)
	{
		float rel_time = crossSceneTime;
		model_view *= RotateX(-10 * sin(rel_time * 10));
	}
	else if (rotato == DOGELOCKHOP)
	{
		float rel_time;
		if (hopSceneTime < 3 || hopSceneTime >= 4)
		{
			rel_time= hopSceneTime;
			model_view *= RotateX(-10 * sin(rel_time * 10));
		}
		else if (hopSceneTime >= 3 && hopSceneTime < 4)
		{
			rel_time = hopSceneTime - 3;
			model_view *= RotateX(30 * sin((360 * rel_time - 90) * DegreesToRadians));
			model_view *= RotateX(30);
		}
	}
	else if (rotato==DOGEGETBONE)
	{
		float rel_time;
		if (boneSceneTime < 3)
		{
			rel_time = boneSceneTime;
			model_view *= RotateX(-10 * sin(rel_time * 10));
		}
	}
	model_view *= Scale(0.3, 0.3, 1);
	model_view *= Translate(0, 0, -1);
	drawCylinder();

	genDogeHead(rotato);
	
	
	mvstack.pop();  //pop front sphere center
}					//at end we are back at torso center

void genDogeBack(float rotato)
{						
	model_view = mvstack.top(); //Back legs
	model_view *= Scale(1.02, 1.02, 0.7);
	model_view *= Translate(0, 0, 1.2);
	drawSphere();
	model_view *= RotateX(-10);
	mvstack.push(model_view);
	model_view *= Translate(0.5, -0.3, 0);
	model_view *= RotateX(-90);
	if (rotato >DOGEINTROTOPSPIN && rotato<DOGEWALLSTOP)
	{
		float rel_time = walkSceneTime - timeToWalk;
		model_view *= RotateX(-10 * sin(rel_time * 10) - 5);
	}
	else if (rotato == DOGEWALLOPEN)
	{
			float rel_time = (unlockSceneTime - 3) ;
			model_view *= RotateX(-10 * sin(rel_time * 10) - 5);
	}
	else if (rotato == DOGECROSSWALL)
	{
		float rel_time = crossSceneTime;
		model_view *= RotateX(-10 * sin(rel_time * 10)-5);
	}
	else if (rotato == DOGELOCKHOP)
	{
		float rel_time;
		if (hopSceneTime < 3 || hopSceneTime >= 4)
		{
			rel_time = hopSceneTime;
			model_view *= RotateX(-10 * sin(rel_time * 10) - 5);
		}
		else if (hopSceneTime >= 3 && hopSceneTime < 4)
		{
			rel_time = hopSceneTime - 3;
			model_view *= RotateX(-30 * sin((360 * rel_time - 90) * DegreesToRadians));
			model_view *= RotateX(-30);
		}
	}
	else if (rotato == DOGEGETBONE)
	{
		float rel_time;
		if (boneSceneTime < 3)
		{
			rel_time = boneSceneTime;
			model_view *= RotateX(-10 * sin(rel_time * 10)-5);
		}
	}
	model_view *= Scale(0.3, 0.3, 1);
	model_view *= Translate(0, 0, -0.7);
	drawCylinder();
	model_view = mvstack.top();
	model_view *= Translate(-0.5, -0.3, 0);
	model_view *= RotateX(-90);
	if (rotato >DOGEINTROTOPSPIN && rotato<DOGEWALLSTOP)
	{
		float rel_time = walkSceneTime - timeToWalk;
		model_view *= RotateX(10 * sin(rel_time * 10) + 5);
	}
	else if (rotato == DOGEWALLOPEN)
	{
			float rel_time = (unlockSceneTime - 3) ;
			model_view *= RotateX(10 * sin(rel_time *10) + 5);
	}
	else if (rotato == DOGECROSSWALL)
	{
		float rel_time = crossSceneTime;
		model_view *= RotateX(10 * sin(rel_time * 10)+5);
	}
	else if (rotato == DOGELOCKHOP)
	{
		float rel_time;
		if (hopSceneTime < 3||hopSceneTime>=4)
		{
			rel_time=hopSceneTime;
			model_view *= RotateX(10 * sin(rel_time * 10) + 5);
		}
		else if (hopSceneTime >= 3 && hopSceneTime < 4)
		{
			rel_time = hopSceneTime - 3;
			model_view *= RotateX(-30 * sin((360 * rel_time - 90) * DegreesToRadians));
			model_view *= RotateX(-30);
		}
	}
	else if (rotato == DOGEGETBONE)
	{
		float rel_time;
		if (boneSceneTime < 3)
		{
			rel_time = boneSceneTime;
			model_view *= RotateX(10 * sin(rel_time * 10)+5);
		}
	}
	model_view *= Scale(0.3, 0.3, 1);
	model_view *= Translate(0, 0, -0.7);
	drawCylinder();

	model_view = mvstack.top();					//Tail wagwagwagwagwag
	model_view *= Translate(0, 0.2, 0.5);
	model_view *= RotateX(-35);
	mvstack.push(model_view);
	for (int x = 0; x < 20; x++)
	{
		model_view = mvstack.top();
		model_view *= RotateY((-10 - 0.9*x) * sin(TIME * 25));
		model_view *= RotateX(-2 * sin(TIME * 10));
		model_view *= Translate(0, 0, 0.5 + 0.05*x);
		model_view *= RotateY((-0.9*x) * sin(TIME * 25));
		model_view *= Scale(0.06, 0.06, 0.03);
		drawCylinder();
	}
	mvstack.pop();
	mvstack.pop();
}

void genDoge(float rotato)
{
	genDogeTorso(rotato);
	genDogeFront(rotato);
	genDogeBack(rotato);

	model_view = mvstack.top(); //back to skewed torso position

}
/**********************************************************
**********************************************************
**********************************************************/
double PREV_TIME = 0;
int frameNumber = 0;
double average;

void display(void)
{
	if (TIME != PREV_TIME) {                   // ensure you don't divide by 0

		double newMeasurement = 1.0 / (TIME - PREV_TIME);       // frames per second

		// average = (average * frameNumber + newMeasurement) / (frameNumber + 1);

		const float decay_period = 10;
		const float a = 2. / (decay_period - 1.);
		average = a * newMeasurement + (1 - a) * average;

		if (frameNumber++ % 10 == 0)
			std::cout << "Average frame rate: " << average << " fps" << std::endl;

		PREV_TIME = TIME;
	}
    // Clear the screen with the background colour (set in myinit)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    model_view = mat4(1.0f);
	mat4 model_trans(1.0f);
	mat4 view_trans(1.0f);

    view_trans *= Translate(0.0f, 0.0f, -15.0f);
    HMatrix r;
    Ball_Value(Arcball,r);

    mat4 mat_arcball_rot(
        r[0][0], r[0][1], r[0][2], r[0][3],
        r[1][0], r[1][1], r[1][2], r[1][3],
        r[2][0], r[2][1], r[2][2], r[2][3],
        r[3][0], r[3][1], r[3][2], r[3][3]);
    view_trans *= mat_arcball_rot;
	view_trans *= Scale(Zoom);
        
    //glUniformMatrix4fv( uView, 1, GL_FALSE, transpose(model_view) );	
	model_view = view_trans;

	jumpSceneTime = TIME - jumpBeginTime;
	walkSceneTime = TIME - walkBeginTime; 
	waddleSceneTime = TIME - waddleBeginTime;
	stopSceneTime = TIME - stopBeginTime;
	unlockSceneTime = TIME - unlockBeginTime;
	crossSceneTime = TIME - crossBeginTime;
	hopSceneTime = TIME - hopBeginTime;
	boneSceneTime = TIME - boneBeginTime;
	if (jumpSceneTime>0 &&jumpSceneTime<timeToJump) //intro scene, doge jumps cutely
	{
		if (jumpSceneTime < (timeToJump - 2.5))
		{
			eye = RotateY(360 / (timeToJump-2.5) * jumpSceneTime)*Translate(4 / (timeToJump-2.5)*jumpSceneTime, -2 / (timeToJump-2.5)*jumpSceneTime, 0)*circleDoge;
			model_view = Angel::LookAt(eye, ref, up);
			genGround(DOGEINTROSPIN);
			genDoge(DOGEINTROSPIN);
		}
		else if (jumpSceneTime >= (timeToJump - 2.5) && jumpSceneTime < (timeToJump - 2))
		{
			eye = Translate(4, -2, 0)*circleDoge;
			model_view = Angel::LookAt(eye, ref, up);
			genGround(DOGEINTROSPIN);
			genDoge(DOGEINTROSPIN);
		}
		else if(jumpSceneTime >= (timeToJump- 2)&&jumpSceneTime<(timeToJump-1)){		
			eye = Translate(4, -2, 0)*circleDoge;
			model_view = Angel::LookAt(eye, ref, up);
			genGround(DOGEINTROSTAND);
			genDoge(DOGEINTROSTAND);
		}
		else if(jumpSceneTime>=(timeToJump-1) && jumpSceneTime<timeToJump){		
			eye = Translate(4, -2, 0)*circleDoge;
			model_view = Angel::LookAt(eye, ref, up);
			genGround(DOGEINTROTOPSPIN);
			genDoge(DOGEINTROTOPSPIN);
		}
	}
	else if(walkSceneTime>0 && walkSceneTime<timeToWalk){
		genGround(DOGEWALKING);
		genDoge(DOGEWALKING);
	}
	else if (waddleSceneTime>0 && waddleSceneTime < timeToWaddle-1)
	{		
		float rel_time = (waddleSceneTime-timeToWaddle);
		eye = Translate((rel_time * -3) - 7, 0, 0)* waddleDoge;		
		model_view = Angel::LookAt(eye, ref, up);
		genGround(DOGEWADDLING);
		genWall(DOGEWADDLING);
		genDoge(DOGEWADDLING);
	}
	else if (stopSceneTime>0 && stopSceneTime < timeToStop)
	{
		genGround(DOGEWALLSTOP);
		model_view *= Translate(5,0,0);
		genWall(DOGEWALLSTOP);
		genDoge(DOGEWALLSTOP);
	}
	else if (unlockSceneTime>0 && unlockSceneTime < timeToUnlock)
	{
		if (unlockSceneTime < 3)
		{
			genLock(DOGEBLOCKLOCK);
			genWall(DOGEBLOCKLOCK);
			genGround(DOGEBLOCKLOCK);
		}
		else{
			genLock(DOGEWALLOPEN);
			genWall(DOGEWALLOPEN);
			genGround(DOGEWALLOPEN);
			model_view *= Translate(-2,0,-20);
			model_view *= RotateY(105);
			genDoge(DOGEWALLOPEN);
		}
	}
	else if (crossSceneTime>0 && crossSceneTime <timeToCross)
	{
		eye = Angel::vec4(0, 20, 3, 1.0);
		model_view = Angel::LookAt(eye, ref, up);
		genWall(DOGECROSSWALL);
		genGround(DOGECROSSWALL);
		model_view *= Translate(5, 0, 0);
		genDoge(DOGECROSSWALL);
		drawCube(); 
	}
	else if (hopSceneTime > 0 && hopSceneTime < timeToHop)
	{
		genGround(DOGELOCKHOP);
		genLock(DOGELOCKHOP);
		model_view *= Translate(10, 0, 5);
		genDoge(DOGELOCKHOP);
	}
	else if (boneSceneTime >0 && boneSceneTime < timeToBone)
	{
		genGround(DOGEGETBONE);
		genWall(DOGEGETBONE);
		genBone();
		genLock(DOGEGETBONE);
		model_view *= Translate(0, 0, 20);
		mvstack.push(model_view);
		model_view *= Scale(2, 2, 2);
		model_view *= Translate(0, 0.5, 0);
		model_view *= RotateY(90);
		genDoge(DOGEGETBONE);		
		model_view = mvstack.top();
		mvstack.pop();
	}
	else {
		set_colour(0,0,0);
		model_view *= Scale(20,20,1);
		drawCube();
	}

    glutSwapBuffers();
    if(Recording == 1)
        FrSaver.DumpPPM(Width, Height);
}

void myReshape(int w, int h)		//handles the window being resized 
{
    glViewport(0, 0, Width = w, Height = h);		
    mat4 projection = Perspective(50.0f, (float)w/(float)h, 1.0f, 1000.0f);
    glUniformMatrix4fv( uProjection, 1, GL_FALSE, transpose(projection) );
}

void myMouseCB(int button, int state, int x, int y)	// start or end mouse interaction
{
    ArcBall_mouseButton = button ;
    if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width-1.0;
        arcball_coords.y = -2.0*(float)y/(float)Height+1.0;
        Ball_Mouse(Arcball, arcball_coords) ;
        Ball_Update(Arcball);
        Ball_BeginDrag(Arcball);

    }
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        Ball_EndDrag(Arcball);
        ArcBall_mouseButton = -1 ;
    }
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        ArcBall_PrevZoomCoord = y ;
    }

    // Tell the system to redraw the window
    glutPostRedisplay() ;
}

// interaction (mouse motion)
void myMotionCB(int x, int y)
{
    if( ArcBall_mouseButton == GLUT_LEFT_BUTTON )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width - 1.0 ;
        arcball_coords.y = -2.0*(float)y/(float)Height + 1.0 ;
        Ball_Mouse(Arcball,arcball_coords);
        Ball_Update(Arcball);
        glutPostRedisplay() ;
    }
    else if( ArcBall_mouseButton == GLUT_RIGHT_BUTTON )
    {
        if( y - ArcBall_PrevZoomCoord > 0 )
            Zoom  = Zoom * 1.03 ;
        else 
            Zoom  = Zoom * 0.97 ;
        ArcBall_PrevZoomCoord = y ;
        glutPostRedisplay() ;
    }
}


void idleCB(void)
{
    if( Animate == 1 )
    {
        // TM.Reset() ; // commenting out this will make the time run from 0
        // leaving 'Time' counts the time interval between successive calls to idleCB
        if( Recording == 0 )
            TIME = TM.GetElapsedTime() ;
        else
            TIME += 0.033 ; // save at 30 frames per second.
        
        eye.x = 20*sin(TIME);
        eye.z = 20*cos(TIME);		
        glutPostRedisplay() ; 
    }
}

int main(int argc, char** argv)		// calls initialization, then hands over control to the event handler, which calls display() whenever the screen needs to be redrawn
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Width,Height);
    glutCreateWindow(argv[0]);
    printf("GL version %s\n", glGetString(GL_VERSION));
#ifndef EMSCRIPTEN
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    myinit();

    glutIdleFunc(idleCB) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCB) ;
    glutMotionFunc(myMotionCB) ;
    instructions();

    glutDisplayFunc(display);
    glutMainLoop();
}