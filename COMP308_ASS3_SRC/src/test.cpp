#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "load/ObjLoader.h"

#include "buffer/Buffer.h"
#include "components/Camera.h"
#include "components/Mesh.h"
#include "components/Texture.h"
#include "pipeline/Pipeline.h"
#include "shader/Shader.h"

#include <glm/gtx/euler_angles.hpp>


using namespace std;


struct Light {
	glm::vec4 LightPosition;
	glm::vec4 La; //Ambient light intensity
	glm::vec4 Ld; //Diffuse light intensity
	glm::vec4 Ls; //Specular light intensity
	glm::vec4 LightDirection;
	float cutoffAngle;
	float cutoffAngleOut;
	float a, b; //padding lol
};


struct LightData {
	Light lights[4];
};

struct MaterialInfo {
	glm::vec4 Ka; //Ambient reflectivity
	glm::vec4 Kd; //Diffuse reflectivity
	glm::vec4 Ks; //Specular reflectivity
	float shininess; //Specular spininess factor
};

struct Rand {
	glm::vec4 rand;
};


struct CurrentTransform {
	glm::mat4 M;
};

struct XForm {

    glm::vec3 Sxyz;
    glm::vec3 Txyz;
    float Theta;
    glm::vec3 Rxyz;

};

typedef std::map<string, Mesh*> TStrMeshMap;
typedef std::pair<string, Mesh*> TStrMeshPair;

typedef std::map<string, Texture*> TStrTexMap;
typedef std::pair<string, Texture*> TStrTexPair;

TStrMeshMap meshMap;
TStrMeshMap::iterator meshIt;

TStrTexMap texMap;
TStrTexMap::iterator texIt;


std::vector<string> objKeySet;
unsigned int currIndex;


int numLights = 4;

float sceneAngle = 0.0f;

bool rotating = false; //flag for whether or not the scene is in the process of rotating
bool mDown = false;

//transform init
glm::mat4 sceneTransform = glm::mat4(1.0f); //used to rotate model matrix
glm::mat4 frameTransform = glm::mat4(1.0f);
glm::vec4 initial = glm::vec4(0.0f);

//cutoff angles for spotlight
float cutoff = 15.0f;
float cutoffOut = 20.0f;

//brushstroke options
float billSize = 0.3f;
float offset = 4.0f;

//shaderstage binds
GLuint plnv;
GLuint plng;
GLuint plnf;
//shaderstage binds for phong
GLuint phongv;
GLuint phongg;
GLuint phongf;

GLuint texv;
GLuint texg;
GLuint texf;

//bind locs
GLuint billSizeLoc;
GLuint offsetLoc;

GLuint texScaleLoc;

Mesh* currentObj = NULL;

GLFWwindow* window = NULL;
int windowWidth, windowHeight;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.01f;

//last mouse position
int lastx, lasty;

//mouse position
float mouseX=0.0f;
float mouseY=0.0f;

// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;

glm::quat mouseDirection;
glm::vec3 keyDirection;

Camera* camPtr = NULL;

void computeAnglesFromInputs(GLFWwindow* window, double xpos, double ypos) {


	glfwSetCursorPos(window, windowWidth/2, windowHeight/2);

	int diffx=(xpos - windowWidth/2); //check the difference between the current x and the last x position
	int diffy=(ypos - windowHeight/2); //check the difference between the current y and the last y position

	mouseX += diffx * mouseSpeed;
	mouseY += diffy * mouseSpeed;


	// Compute new orientation
	horizontalAngle = (float)diffx*mouseSpeed;
	verticalAngle = -1*(float)diffy*mouseSpeed;

	lastx=xpos; //set lastx to the current x position
	lasty=ypos; //set lasty to the current y position

	glm::vec3 camera_up = glm::vec3(0,1,0);
	glm::vec3 camera_direction = glm::vec3(0,0,1);// * camPtr->cameraAngle();
	glm::vec3 axis = glm::cross(camera_direction, camera_up);

	//determine heading quaternion from the camera up vector and the heading angle
	glm::quat heading_quat = glm::angleAxis(mouseX, glm::vec3(0,1,0));
	//compute quaternion for pitch based on the camera pitch angle
	glm::quat pitch_quat = glm::angleAxis(mouseY, glm::vec3(1,0,0));

	//add the two quaternions
	glm::quat temp = pitch_quat * heading_quat;

	mouseDirection  = temp;

}

void error_callback(int error, const char* description) {
	cerr << description << endl;
}

void checkGLError() {
	int error = glGetError();
	string estr;
	switch (error) {
	case GL_NO_ERROR:
		estr = "GL_NO_ERROR";
		break;
	case GL_INVALID_ENUM:
		estr = "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		estr = "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		estr = "GL_INVALID_OPERATION";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		estr = "GL_INVALID_FRAMEBUFFER_OPERATION";
		break;
	case GL_OUT_OF_MEMORY:
		estr = "GL_OUT_OF_MEMORY";
		break;
	case GL_STACK_UNDERFLOW:
		estr = "GL_STACK_UNDERFLOW";
		break;
	case GL_STACK_OVERFLOW:
		estr = "GL_STACK_OVERFLOW";
		break;
	}
    if (error) cout << "error = " << estr << endl;

}

void setLightValues(int i,
		const Buffer<LightData>& lightData,
		const glm::vec4& pos,
		const glm::vec4& la,
		const glm::vec4& ld,
		const glm::vec4& ls,
		const glm::vec4& direction,
		const float& cutoffAngle,
		const float& cutoffAngleOut) {

	lightData.data->lights[i].LightPosition = pos;
	lightData.data->lights[i].La = la;
	lightData.data->lights[i].Ld = ld;
	lightData.data->lights[i].Ls = ls;
	lightData.data->lights[i].LightDirection = direction;
	lightData.data->lights[i].cutoffAngle = cutoffAngle;
	lightData.data->lights[i].cutoffAngleOut = cutoffAngleOut;

}

void setMaterialValues(
		const Buffer<MaterialInfo>& materialData,
		const glm::vec4& ka,
		const glm::vec4& kd,
		const glm::vec4& ks,
		const float& shine) {

	materialData.data->Ka = ka;
	materialData.data->Kd = kd;
	materialData.data->Ks = ks;
	materialData.data->shininess = shine;

}

void rotateScene() {
	sceneAngle += 1.0f;
	if (sceneAngle < 360.0f) {
		sceneTransform *= glm::rotate(1.0f, 0.0f, 1.0f, 0.0f);
	} else {
		rotating = false;
		sceneAngle = 0;
	}
}

void setBillProperties(const GLuint& stage, const GLuint& billLoc, const GLuint& offLoc, const Mesh* obj) {
	glProgramUniform1f(stage, billLoc, obj->billSize); //sets scale of billboard
	glProgramUniform1f(stage, offLoc, obj->offset); //sets offset of billboard
}

void setObjProperties(const MaterialInfo& material, Buffer<MaterialInfo>& matInfo, Pipeline& pipeline) {
	//set up billboards
	setBillProperties(plng, billSizeLoc, offsetLoc, meshIt->second);
	//set material values
	setMaterialValues(matInfo, material.Ka, material.Kd, material.Ks, material.shininess); //set mat props
	matInfo.update(); //update material info
	//set texture
	texIt = texMap.find(meshIt->second->stroke);
	texIt->second->Bind(pipeline.get("frameTexture")); //select stroke
}

void setObjPropertiesPhong(const MaterialInfo& material, Buffer<MaterialInfo>& matInfo, Pipeline& pipeline) {

	//set material values
	setMaterialValues(matInfo, material.Ka, material.Kd, material.Ks, material.shininess); //set mat props
	matInfo.update(); //update material info

}

void setObjPropertiesTex(const MaterialInfo& material, Buffer<MaterialInfo>& matInfo, Pipeline& pipeline) {

	//set material values
	setMaterialValues(matInfo, material.Ka, material.Kd, material.Ks, material.shininess); //set mat props
	matInfo.update(); //update material info
	glProgramUniform1f(texf, texScaleLoc, 2.0f); //sets scale of texture
	texIt = texMap.find(meshIt->second->stroke);
	texIt->second->Bind(pipeline.get("frameTexture")); //select stroke

}

void setTransforms(glm::mat4 matrix, Buffer<CurrentTransform>& xform, const XForm& x) {

	xform.data->M = glm::translate(matrix, x.Txyz);
	xform.data->M = glm::rotate(xform.data->M, x.Theta, x.Rxyz);
	xform.data->M = glm::scale(xform.data->M, x.Sxyz);
	xform.update();

}

void drawPainterly(glm::mat4 matrix, const std::string& name, const MaterialInfo& matData, const XForm& xData,
		Buffer<MaterialInfo>& materialInfo, Pipeline& pipeline,
		Buffer<CurrentTransform>& ct) {
	//get mesh
	meshIt = meshMap.find(name);
	//set up billboards & mat properties
	setObjProperties(matData, materialInfo, pipeline);
	//update transforms
	setTransforms(matrix, ct, xData);
	//finally draw
	meshIt->second->Draw();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
		if(currIndex != objKeySet.size()) {
			std::string key = objKeySet.at(currIndex);
			currentObj = meshMap.find(key)->second;
			cout << key << "\n";
			currIndex++;
		} else {
			currIndex = 0;
		}
    }
}

int main(int argc, char *argv[]) {
	windowWidth = 1024;
	windowHeight = 768;

	/*
	 * initialise glfw window first
	 */
	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwSetErrorCallback(error_callback);
	window = glfwCreateWindow(windowWidth, windowHeight, "Shader", NULL, NULL);
	glfwMakeContextCurrent(window);

	/*
	 * initialize glew after
	 */
    if (glewInit() != GLEW_OK) {
    	cerr << "Failed to initialize GLEW" << endl;
        exit(EXIT_FAILURE);
    }

    /*
     * Must set special key callback for switching between objects
     */
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPos(window, windowWidth/2, windowHeight/2);
    glfwSetCursorPosCallback (window, computeAnglesFromInputs);

    srand( time( NULL ) );

    /*
     * define buffers
     */
    Buffer<CurrentTransform> ct(GL_SHADER_STORAGE_BUFFER);
    Buffer<LightData>lightData(GL_SHADER_STORAGE_BUFFER);
    Buffer<Rand> randData(GL_SHADER_STORAGE_BUFFER);
    Buffer<MaterialInfo> materialInfo(GL_SHADER_STORAGE_BUFFER);

    //Obj to render sent to map
	meshMap.insert(TStrMeshPair("teapot",new Mesh("assets/teapot.obj","brush5", 0.3, 3.5)));
	meshMap.insert(TStrMeshPair("pitcher",new Mesh("assets/pitcher.obj","brush5", 0.3, 1.6)));
	meshMap.insert(TStrMeshPair("rose",new Mesh("assets/rose.obj","brush1", 0.4, 4.0)));
	meshMap.insert(TStrMeshPair("table",new Mesh("assets/nicetable.obj","brush5", 0.4, 1.6)));
	meshMap.insert(TStrMeshPair("frame",new Mesh("assets/frame.obj","wood", 0.0, 0.0))); //0,0 no billboard

	//initialise obj key set
	currIndex = 0;
	currentObj = meshMap.find("teapot")->second; //set default selected object

	for(meshIt = meshMap.begin(); meshIt != meshMap.end(); ++meshIt) {
	  objKeySet.push_back(meshIt->first);
	  cout << meshIt->first << "\n";
	}

	//Paint Brush texture sent to map
    texMap.insert(TStrTexPair("brush1", new Texture("assets/brushes/brush1.png")));
    texMap.insert(TStrTexPair("brush2", new Texture("assets/brushes/brush2.png")));
    texMap.insert(TStrTexPair("brush3", new Texture("assets/brushes/brush3.png")));
    texMap.insert(TStrTexPair("brush4", new Texture("assets/brushes/brush4.png")));
    texMap.insert(TStrTexPair("brush5", new Texture("assets/brushes/brush5.png")));
    texMap.insert(TStrTexPair("brush6", new Texture("assets/brushes/brush6.png")));
    texMap.insert(TStrTexPair("brush7", new Texture("assets/brushes/brush7.png")));
    texMap.insert(TStrTexPair("brush8", new Texture("assets/brushes/brush8.png")));
    texMap.insert(TStrTexPair("brush9", new Texture("assets/brushes/brush9.png")));
    texMap.insert(TStrTexPair("wood", new Texture("assets/wood.jpg")));



    // Init Pipeline
    Shader plain_vert("glsl/paint.vert", GL_VERTEX_SHADER);
    Shader plain_geom("glsl/paint.geom", GL_GEOMETRY_SHADER);
    Shader plain_frag("glsl/paint.frag", GL_FRAGMENT_SHADER);

    // Init Pipeline for phong shaded Room
    Shader phong_vert("glsl/phong.vert", GL_VERTEX_SHADER);
    Shader phong_geom("glsl/phong.geom", GL_GEOMETRY_SHADER);
    Shader phong_frag("glsl/phong.frag", GL_FRAGMENT_SHADER);

    // Init Pipeline for textured shaded Frame
    Shader textured_vert("glsl/textured.vert", GL_VERTEX_SHADER);
    Shader textured_geom("glsl/textured.geom", GL_GEOMETRY_SHADER);
    Shader textured_frag("glsl/textured.frag", GL_FRAGMENT_SHADER);

    Pipeline pipeline;
    //initialise pipeline programs
    plnv = pipeline.makeProgram(plain_vert, true);
    plng = pipeline.makeProgram(plain_geom, true);
    plnf = pipeline.makeProgram(plain_frag, true);

    //initialise pipeline programs for phong models
    phongv = pipeline.makeProgram(phong_vert, true);
    phongg = pipeline.makeProgram(phong_geom, true);
    phongf = pipeline.makeProgram(phong_frag, true);

    //initialise pipeline programs for textured models
	texv = pipeline.makeProgram(textured_vert, true);
	texg = pipeline.makeProgram(textured_geom, true);
	texf = pipeline.makeProgram(textured_frag, true);


    /*
     * camera viewpoint object
     */
    Camera camera;
    camPtr = &camera;
    camera.resize(windowWidth, windowHeight);


    /*
     * make bindings to the shader.update();
     */
	camera.properties()->bind(pipeline.get("Camera"));
	ct.bind(pipeline.get("CurrentTransform"));

	/*
	 * spotlight
	 */
	glm::vec4 direction(0.0f,0.0f,0.0f,1.0f); //general light direction towards the center of the table
	glm::vec4 pos(0.0f, 15.0f, 20.0f, 1.0f); //position
	glm::vec4 la(0.7f, 0.7f, 0.7f, 0.0f); //ambient
	glm::vec4 ld(0.7f, 0.7f, 0.7f, 0.0f); //diffuse
	glm::vec4 ls(0.7f, 0.7f, 0.7f, 0.0f); //specular

	/*
	 * ambient
	 */

	glm::vec4 pos1(0.0f, 15.0f, 20.0f, 1.0f); //position
	glm::vec4 la1(0.1f, 0.1f, 0.0f, 0.0f); //ambient
	glm::vec4 ld1(0.6f, 0.6f, 0.6f, 0.0f); //diffuse
	glm::vec4 ls1(1.0f, 1.0f, 1.0f, 0.0f); //specular

	setLightValues(0,lightData,pos,la,ld,ls,direction,cutoff,cutoffOut);
	setLightValues(1,lightData,pos1,la1,ld1,ls1,direction,360,360);
    lightData.bind(pipeline.get("LightData"));
    randData.bind(pipeline.get("Rand"));

    billSizeLoc = pipeline.get("billSize");
    offsetLoc = pipeline.get("offset");

    //scale of texture on texture phong shader
    texScaleLoc = pipeline.get("texScale");

    //Set up material values: ambient, diffuse, specular, shine
    MaterialInfo tableMat{glm::vec4(0.7f, 0.6f, 0.6f, 0.0f), glm::vec4(0.7f, 0.4f, 0.4f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),7.0f};
    MaterialInfo teapotMat{glm::vec4(0.5f, 0.5f, 0.9f, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f), glm::vec4(0.8f, 0.9f, 0.9f, 1.0f),10.0f};
    MaterialInfo pitcherMat{glm::vec4(0.4f, 0.4f, 0.4f, 0.0f), glm::vec4(0.9f, 0.9f, 0.2f, 1.0f), glm::vec4(0.9f, 0.9f, 0.1f, 1.0f),15.0f};
    MaterialInfo roseMat{glm::vec4(0.4f, 0.1f, 0.1f, 0.0f), glm::vec4(0.9f, 0.2f, 0.2f, 1.0f), glm::vec4(0.9f, 0.2f, 0.2f, 1.0f),1.0f};
    MaterialInfo blueRoseMat{glm::vec4(0.1f, 0.1f, 0.4f, 0.0f), glm::vec4(0.2f, 0.2f, 0.9f, 0.0f), glm::vec4(0.2f, 0.2f, 0.9f, 1.0f),1.0f};
    MaterialInfo greenRoseMat{glm::vec4(0.1f, 0.4f, 0.1f, 0.0f), glm::vec4(0.2f, 0.9f, 0.2f, 1.0f), glm::vec4(0.2f, 0.9f, 0.2f, 1.0f), 1.0f};
    MaterialInfo frameMat{glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),10.0f};


    float offsetY = 5.0f;
    float paintingAngle = 30.0f;
    float offsetZ = 20.0f;
    glm::vec3 tableCenter = glm::vec3(0.0f, -3.5f-offsetY, -5.0f-offsetZ);
    //transform info: scale, transform, angle, rotation axis
    XForm tableXform{glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0,0,0) , 0.0f, glm::vec3(1.0f, 1.0f, 1.0f)};
    XForm teapotXform{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-8.0f,1.5f,8.0f), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f)};
    XForm pitcherXform{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 2.0f, 4.0f), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f)};
    XForm roseXform{glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, 13.0f, 1.1f+4.5f), 40.0f, glm::vec3(1.0f, 0.0f, 0.0f)};
    XForm rose1Xform{glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, 13.0f, -1.1f+4.5f), -40.0f, glm::vec3(1.0f, 0.0f, 0.0f)};
    XForm roseBlueXform{glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(1.1f, 13.0f, 0.0f+4.5f), -40.0f, glm::vec3(0.0f, 0.0f, 1.0f)};
    XForm roseGreenXform{glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(-1.1f, 13.0f, 0.0f+4.5f), 40.0f, glm::vec3(0.0f, 0.0f, 1.0f)};
    XForm frameXform{glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, -offsetY, 10.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)};

    materialInfo.bind(pipeline.get("MaterialInfo"));

    sceneTransform = glm::translate(glm::mat4(1.0f), tableCenter);
    sceneTransform *= glm::rotate(15.0f, 1.0f, 0.0f, 0.0f);
    sceneTransform *= glm::rotate(15.0f, 0.0f, 1.0f, 0.0f);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window, windowWidth/2, windowHeight/2);
    mouseDirection = glm::quat(1,0,0,0);

	while (!glfwWindowShouldClose(window))
	{
		/*
		 * Main window get frame buffer size and set viewport
		 */
		glfwMakeContextCurrent(window);
	    int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//apply first person rotation
		camera.rotate(mouseDirection);

		/*
		 * Keyboard Input
		 */
		if (glfwGetKey(window, GLFW_KEY_X)) {
			lightData.data->lights[0].cutoffAngle += 0.1;
			lightData.data->lights[0].cutoffAngleOut += 0.1;
		} else if (glfwGetKey(window, GLFW_KEY_Z)) {
			lightData.data->lights[0].cutoffAngle -= 0.1;
			lightData.data->lights[0].cutoffAngleOut -= 0.1;
		} else if (glfwGetKey(window, GLFW_KEY_T)) {
			rotating = true;
		} else if(glfwGetKey(window, GLFW_KEY_KP_ADD)) {
			currentObj->billSize += 0.01;
		} else if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT)) {
			currentObj->billSize -= 0.01;
		} else if(glfwGetKey(window, GLFW_KEY_KP_MULTIPLY)) {
			currentObj->offset += 0.1;
		} else if(glfwGetKey(window, GLFW_KEY_KP_DIVIDE)) {
			currentObj->offset -= 0.1;
		} else if(glfwGetKey(window, GLFW_KEY_KP_1)) {
			currentObj->stroke = "brush1";
		} else if(glfwGetKey(window, GLFW_KEY_KP_2)) {
			currentObj->stroke = "brush2";
		} else if(glfwGetKey(window, GLFW_KEY_KP_3)) {
			currentObj->stroke = "brush3";
		} else if(glfwGetKey(window, GLFW_KEY_KP_4)) {
			currentObj->stroke = "brush4";
		} else if(glfwGetKey(window, GLFW_KEY_KP_5)) {
			currentObj->stroke = "brush5";
		} else if(glfwGetKey(window, GLFW_KEY_KP_6)) {
			currentObj->stroke = "brush6";
		} else if(glfwGetKey(window, GLFW_KEY_KP_7)) {
			currentObj->stroke = "brush7";
		} else if(glfwGetKey(window, GLFW_KEY_KP_8)) {
			currentObj->stroke = "brush8";
		} else if(glfwGetKey(window, GLFW_KEY_KP_9)) {
			currentObj->stroke = "brush9";
		} else if(glfwGetKey(window, GLFW_KEY_UP)) {
			camera.translate(glm::vec3(0,0,1) * camera.cameraAngle());
		} else if(glfwGetKey(window, GLFW_KEY_DOWN)) {
			camera.translate(glm::vec3(0,0,-1) * camera.cameraAngle());
		} else if(glfwGetKey(window, GLFW_KEY_LEFT)) {
			camera.translate(glm::vec3(1,0,0) * camera.cameraAngle());
		} else if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
			camera.translate(glm::vec3(-1,0,0) * camera.cameraAngle());
		}

		/*
		 * update camera and bind
		 */

		//rotate scene if selected to
		if(rotating) {
			rotateScene();
		}

	    randData.data->rand = initial;
	    randData.update();

		lightData.update();
		materialInfo.update();
		camera.update(0.01);



		/*
		 * Draw Phong'd
		 */

		glBindProgramPipeline(pipeline.name);
	    pipeline.setStage(texv, GL_VERTEX_SHADER_BIT);
	    pipeline.setStage(texg, GL_GEOMETRY_SHADER_BIT);
	    pipeline.setStage(texf, GL_FRAGMENT_SHADER_BIT);

	    ct.data->M = glm::mat4(1.0f); //reset
	    frameTransform = glm::mat4(1.0f);
	    meshIt = meshMap.find("frame");
		//set  mat properties
		setObjPropertiesTex(frameMat, materialInfo, pipeline);
		//update transforms
		setTransforms(frameTransform,ct,frameXform);
		//finally draw
		meshIt->second->Draw();


	    /*
	     * Draw painterly
	     */

	    pipeline.setStage(plnv, GL_VERTEX_SHADER_BIT);
	    pipeline.setStage(plng, GL_GEOMETRY_SHADER_BIT);
	    pipeline.setStage(plnf, GL_FRAGMENT_SHADER_BIT);

	    //get mesh and paint, bitches.
	    ct.data->M = glm::mat4(1.0f); //reset

		drawPainterly(sceneTransform, "teapot",teapotMat, teapotXform, materialInfo, pipeline, ct);
		drawPainterly(sceneTransform, "pitcher",pitcherMat, pitcherXform, materialInfo, pipeline, ct);
		drawPainterly(sceneTransform, "table",tableMat, tableXform, materialInfo, pipeline, ct);
		drawPainterly(sceneTransform, "rose",roseMat, roseXform, materialInfo, pipeline, ct);
		drawPainterly(sceneTransform, "rose",blueRoseMat,roseBlueXform , materialInfo, pipeline, ct);
		drawPainterly(sceneTransform, "rose",roseMat, rose1Xform, materialInfo, pipeline, ct);
		drawPainterly(sceneTransform, "rose",greenRoseMat, roseGreenXform, materialInfo, pipeline, ct);


		glfwSwapBuffers(window);
		glFlush();
		checkGLError();
		glfwPollEvents();

	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}








