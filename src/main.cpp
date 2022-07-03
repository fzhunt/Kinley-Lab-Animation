/**********************************/
/* Lighting					      https://youtu.be/_p7K2pnmy00?t=852
   (C) Bedrich Benes 2021         
   Diffuse and specular per fragment.
   bbenes@purdue.edu               */
/**********************************/

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <string.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <string>
#include <vector>			//Standard template library class
#include <GL/glew.h>
#include <GL/glut.h>
//glm
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/half_float.hpp>
#include "shaders.h"    
#include "shapes.h"    
#include "lights.h"    

#include "tiny_obj_loader.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#include "Row.cpp"

#pragma warning(disable : 4996)
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glut32.lib")

using namespace std;

bool needRedisplay=false;
ShapesC* sphere;
ShapesC* stick;
bool unpaused = true;
float speed;

/*
* use the view function, they're all supposed to be points, so destination is the point that'll be ending
* make sure that the bird is originally facing positive z, or if that doesn't work do x
* can pull things out of blendr pretty easily, maybe maya would be easier to learn
* view = glm::lookAt(glm::vec3(420.f, 420.f, 420.f),//eye
		glm::vec3(0, 0, 0),  //destination
		glm::vec3(0, 1, 0)); //up
 * if it happens to be 
*/


//shader program ID
GLuint shaderProgram;
GLfloat ftime=0.f;
float dt = 0.7f;
glm::mat4 view=glm::mat4(1.0);
glm::mat4 proj=glm::perspective(80.0f,//fovy
				  		        1.0f,//aspect
						        0.01f,1000.f); //near, far
class ShaderParamsC
{
public:
	GLint modelParameter;		//modeling matrix
	GLint modelViewNParameter;  //modeliview for normals
	GLint viewParameter;		//viewing matrix
	GLint projParameter;		//projection matrix
	//material
	GLint kaParameter;			//ambient material
	GLint kdParameter;			//diffuse material
	GLint ksParameter;			//specular material
	GLint shParameter;			//shinenness material
} params;

struct delete_ptr {
	template <typename P>
	void operator () (P p) {
		delete p;
	}
};

LightC light;

//the main window size
GLint wWindow=900;
GLint hWindow=900;

float sh=1;

/*********************************
Some OpenGL-related functions
**********************************/

//called when a window is reshaped
void Reshape(int w, int h)
{
  glViewport(0,0,w, h);       
  glEnable(GL_DEPTH_TEST);
  wWindow=w;
  hWindow=h;
}

//the main rendering function
void RenderObjects()
{
	const int range = 3;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//set the projection and view once for the scene
	glUniformMatrix4fv(params.projParameter, 1, GL_FALSE, glm::value_ptr(proj));
	//view=glm::lookAt(glm::vec3(25*sin(ftime/40.f),5.f,15*cos(ftime/40.f)),//eye
	//			     glm::vec3(0,0,0),  //destination
	//			     glm::vec3(0,1,0)); //up
	view = glm::lookAt(glm::vec3(10.f, 10.f, 10.f),//eye
		glm::vec3(0, 0, 0),  //destination
		glm::vec3(0, 1, 0)); //up
	glUniformMatrix4fv(params.viewParameter, 1, GL_FALSE, glm::value_ptr(view));
	//set the light
	static glm::vec4 pos = glm::vec4(-10, 10, 10, 1);
	light.SetPos(pos);
	light.SetShaders();
	glutPostRedisplay();

	glm::mat4 m = glm::mat4(1.0);
	m = glm::scale(m, glm::vec3(1.0f));
	stick->SetModel(m);
	glm::mat3 modelViewN = glm::mat3(view * m);
	modelViewN = glm::transpose(glm::inverse(modelViewN));
	stick->SetModelViewN(modelViewN);
	stick->Render();
}

void renderGUI() {
	ImGui::Begin("-------------- Animation Start --------------");                         

	ImGui::SliderFloat("Animation Speed", &speed, -3.0f, 3.0f);

	if (ImGui::Button("Reset to Defaults")) {
		speed = 1.0f;
	}
	if (ImGui::Button("Pause")) {
		unpaused = !unpaused;
	}
	
	if (ImGui::Button("Exit")) {
		exit(0);
	}
	ImGui::End();
}
	
void Idle(void)
{
	/*ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();

  glClearColor(0.1,0.1,0.1,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  ftime+=0.05;
  glUseProgram(shaderProgram);

  renderGUI();
  RenderObjects();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glutSwapBuffers(); */ 
}

void Display(void)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();

	renderGUI();

	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();

	

	glClearColor(0.1, 0.1, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ftime += 0.05;
	glUseProgram(shaderProgram);
	RenderObjects();
	

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glutSwapBuffers();
}

//keyboard callback
void Kbd(unsigned char a, int x, int y)
{
	switch(a)
	{
 	  case 27 : exit(0);break;
	  case 'p': 
	  case 'P':
		  unpaused = !unpaused;
		  speed = 0;
		  break;
	  case 'g': 
	  case 'G': {stick->SetKd(glm::vec3(0,1,0));break;} // these were all sphere before cone was added
	  case 'b': 
	  case 'B': {stick->SetKd(glm::vec3(0,0,1));break;}
	  case 'w': 
	  case 'W': {stick->SetKd(glm::vec3(0.7,0.7,0.7)); break;}
	  case '+': {stick->SetSh(sh+=1);break;}
	  case '-': {stick->SetSh(sh-=1);if (sh<1) sh=1;break;}
	}
	glutPostRedisplay();
}


//special keyboard callback
void SpecKbdPress(int a, int x, int y)
{
   	switch(a)
	{
 	  case GLUT_KEY_LEFT  : 
		  {
			  break;
		  }
	  case GLUT_KEY_RIGHT : 
		  {
			break;
		  }
 	  case GLUT_KEY_DOWN    : 
		  {
			break;
		  }
	  case GLUT_KEY_UP  :
		  {
			break;
		  }

	}
	glutPostRedisplay();
}

//called when a special key is released
void SpecKbdRelease(int a, int x, int y)
{
	switch(a)
	{
 	  case GLUT_KEY_LEFT  : 
		  {
			  break;
		  }
	  case GLUT_KEY_RIGHT : 
		  {
			  break;
		  }
 	  case GLUT_KEY_DOWN  : 
		  {
			break;
		  }
	  case GLUT_KEY_UP  :
		  {
			break;
		  }
	}
	glutPostRedisplay();
}


void Mouse(int button,int state,int x,int y)
{
	//cout << "Location is " << "[" << x << "'" << y << "]" << endl;
}


void InitializeProgram(GLuint *program)
{
	std::vector<GLuint> shaderList;

//load and compile shaders 	
	shaderList.push_back(CreateShader(GL_VERTEX_SHADER,   LoadShader("shaders/phong.vert")));
	shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, LoadShader("shaders/phong.frag")));

//create the shader program and attach the shaders to it
	*program = CreateProgram(shaderList);

//delete shaders (they are on the GPU now)
	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	params.modelParameter=glGetUniformLocation(*program,"model");
	params.modelViewNParameter=glGetUniformLocation(*program,"modelViewN");
	params.viewParameter =glGetUniformLocation(*program,"view");
	params.projParameter =glGetUniformLocation(*program,"proj");
	//now the material properties
	params.kaParameter=glGetUniformLocation(*program,"mat.ka");
	params.kdParameter=glGetUniformLocation(*program,"mat.kd");
	params.ksParameter=glGetUniformLocation(*program,"mat.ks");
	params.shParameter=glGetUniformLocation(*program,"mat.sh");
	//now the light properties
	light.SetLaToShader(glGetUniformLocation(*program,"light.la"));
	light.SetLdToShader(glGetUniformLocation(*program,"light.ld"));
	light.SetLsToShader(glGetUniformLocation(*program,"light.ls"));
	light.SetLposToShader(glGetUniformLocation(*program,"light.lPos"));
}

void InitShapes(ShaderParamsC *params)
{
//create one unit sphere in the origin
	stick = new ModelC("ballandstick.obj");
	stick->SetKa(glm::vec3(0.1, 0.1, 0.1));
	stick->SetKs(glm::vec3(0.7, 0.7, 0.7));
	stick->SetKd(glm::vec3(0.7, 1, 0.7));
	stick->SetSh(200);
	stick->SetModel(glm::mat4(1.0));
	stick->SetModelMatrixParamToShader(params->modelParameter);
	stick->SetModelViewNMatrixParamToShader(params->modelViewNParameter);
	stick->SetKaToShader(params->kaParameter);
	stick->SetKdToShader(params->kdParameter);
	stick->SetKsToShader(params->ksParameter);
	stick->SetShToShader(params->shParameter);
}

void readSpreadsheet(string filename) {
	ifstream infile;
	string line;
	infile.open(filename, ios::in);
	getline(infile, line);
	while (!getline(infile, line).eof()) {
		//time, then pitch, roll, yaw
		double time, pitch, roll, yaw;
		
	}
}

int main(int argc, char **argv)
{ 
  glutInitDisplayString("stencil>=2 rgb double depth samples");
  glutInit(&argc, argv);
  glutInitWindowSize(wWindow,hWindow);
  glutInitWindowPosition(500,100);
  glutCreateWindow("Model View Projection GLSL");
  GLenum err = glewInit();
  if (GLEW_OK != err){
   fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  glutMouseFunc(Mouse);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(Kbd); //+ and -
  glutSpecialUpFunc(SpecKbdRelease); //smooth motion
  glutSpecialFunc(SpecKbdPress);
  InitializeProgram(&shaderProgram);
  InitShapes(&params);


  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  ImGui::StyleColorsDark();

  ImGui_ImplGLUT_Init();
  ImGui_ImplGLUT_InstallFuncs();
  ImGui_ImplOpenGL3_Init();

  glutMainLoop();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGLUT_Shutdown();
  ImGui::DestroyContext();

  return 0;        
}
	