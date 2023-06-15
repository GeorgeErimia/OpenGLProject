#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "CommonValues.h"

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "Material.h"

#include "Model.h"

#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
uniformSpecularIntensity = 0, uniformShininess = 0,
uniformDirectionalLightTransform = 0, uniformOmniLightPos = 0, uniformFarPlane = 0;

Window mainWindow;
std::vector<Mesh*> meshList;

std::vector<Shader> shaderList;
Shader directionalShadowShader;
Shader omniShadowShader;

Camera camera;

Texture plainTexture;
Texture waterTexture;

Material shinyMaterial;
Material dullMaterial;

Model mountains;
Model ocean;
Model submarine;

DirectionalLight mainLight;

Skybox skybox;

unsigned int spotLightCount = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// submarine coordinates
GLfloat submarineHeight = 0.0f;
glm::vec3 lastSubmarinePos = glm::vec3(-7.0f, -2.2f, 0.0f);
glm::vec3 submarinePos = glm::vec3(-7.0f, -2.2f, 0.0f);
GLfloat submarineAngle = 0.0f;
glm::vec3 submarineDirection(0.0f, 0.0f, 1.0f);

GLfloat turnAngle = 0.0f;
glm::vec3 waterPos = glm::vec3(0.0f, -2.0f, 0.0f);
glm::vec3 mountainPos = glm::vec3(0.0f, 0.0f, 0.0f);

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-100.0f, 0.0f, -100.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		100.0f, 0.0f, -100.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-100.0f, 0.0f, 100.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		100.0f, 0.0f, 100.0f,	10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	Mesh* floor = new Mesh();
	floor->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(floor);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map.vert", "Shaders/directional_shadow_map.frag");
}

void setSubmarineKeyControl(bool* keys)
{
	GLfloat velocity = 0.5f * deltaTime;

	if (keys[GLFW_KEY_W])
	{
		lastSubmarinePos = submarinePos;
		// Move the submarine forwards
		submarinePos += velocity * submarineDirection;
	}

	if (keys[GLFW_KEY_S])
	{
		lastSubmarinePos = submarinePos;
		// Move the submarine backwards
		submarinePos -= velocity * submarineDirection;
	}

	if (keys[GLFW_KEY_LEFT_SHIFT])
	{
		lastSubmarinePos = submarinePos;
		// Lower the submarine into the water
		if (!(submarinePos.y - velocity < -3.0f))
			submarinePos.y -= velocity;
	}

	if (keys[GLFW_KEY_SPACE])
	{
		lastSubmarinePos = submarinePos;
		// Lift the submarine up
		if(!(submarinePos.y + velocity > -2.2f))
			submarinePos.y += velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		turnAngle -= velocity;
		// Turn the submarine to the left
		submarineDirection = glm::vec3(-sin(turnAngle), 0.0f, cos(turnAngle));
	}

	if (keys[GLFW_KEY_D])
	{
		turnAngle += velocity;
		// Turn the submarine to the right
		submarineDirection = glm::vec3(-sin(turnAngle), 0.0f, cos(turnAngle));
	}
}

void RenderScene()
{
	glm::mat4 model(1.0f);

	// Render Water
	model = glm::translate(model, waterPos);
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	// Render Mountains
	model = glm::mat4(1.0f);
	model = glm::translate(model, mountainPos);
	model = glm::scale(model, glm::vec3(0.2f / SCALE_FACTOR, 0.2f / SCALE_FACTOR, 0.2f / SCALE_FACTOR));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	mountains.RenderModel();

	// Render Submarine Animation

	model = glm::mat4(1.0f);
	model = glm::translate(model, submarinePos);
	model = glm::rotate(model, -turnAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f / SCALE_FACTOR, 1.0f / SCALE_FACTOR, 1.0f / SCALE_FACTOR));

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	waterTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	submarine.RenderModel();

	GLfloat velocity = 0.5f * deltaTime;
}

void DirectionalShadowMapPass(DirectionalLight* light)
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->getShadowMap()->GetShadowWidth(), light->getShadowMap()->GetShadowHeight());

	light->getShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();
	auto lightTransform = light->CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&lightTransform);

	directionalShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	glViewport(0, 0, 1600, 900);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);

	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);
	auto mainLightTransform = mainLight.CalculateLightTransform();
	shaderList[0].SetDirectionalLightTransform(&mainLightTransform);

	mainLight.getShadowMap()->Read(GL_TEXTURE2);
	shaderList[0].SetTexture(1);
	shaderList[0].SetDirectionalShadowMap(2);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;

	shaderList[0].Validate();

	RenderScene();
}

int main()
{
	mainWindow = Window(1600, 900);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 170.0f, 0.0f, 5.0f, 0.1f);

	// Load Textures for in-house built objects
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	waterTexture = Texture("Textures/ocean.png");
	waterTexture.LoadTexture();

	// Two materials (different shininess)
	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	// Mountains
	mountains = Model();
	mountains.LoadModel("Models/mountain.obj");

	// Submarine
	submarine = Model();
	submarine.LoadModel("Models/submarine.obj");

	// Ocean
	ocean = Model();
	ocean.LoadModel("Models/ocean.obj");


	// Light (MAIN)
	mainLight = DirectionalLight(2048, 2048,
		0.7f, 0.7f, 0.8f,
		0.1f, 0.9f,
		-10.0f, -12.0f, 18.5f);


	// Skybox
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/sky_right.tga");
	skyboxFaces.push_back("Textures/Skybox/sky_left.tga");
	skyboxFaces.push_back("Textures/Skybox/sky_up.tga");
	skyboxFaces.push_back("Textures/Skybox/sky_down.tga");
	skyboxFaces.push_back("Textures/Skybox/sky_back.tga");
	skyboxFaces.push_back("Textures/Skybox/sky_front.tga");

	skybox = Skybox(skyboxFaces);


	//// Ids
	//GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
	//	uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		// Calculate delta time
		GLfloat now = glfwGetTime(); // SDL_GetPerformanceCounter();
		deltaTime = now - lastTime; // (now - lastTime)*1000/SDL_GetPerformanceFrequency();
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();

		// // Use key control when using old mouse control
		//camera.keyControl(mainWindow.getsKeys(), deltaTime);

		setSubmarineKeyControl(mainWindow.getsKeys());
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), submarinePos, lastSubmarinePos);

		// // Old movement (not focused on object)
		//camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());


		printf("%f %f %f\n", camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		DirectionalShadowMapPass(&mainLight);
		RenderPass(camera.calculateViewMatrix(), projection);

		mainWindow.swapBuffers();
	}

	return 0;
}
