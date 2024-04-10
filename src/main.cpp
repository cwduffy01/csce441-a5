#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Thing.h"
#include "helpers.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Material> material;
shared_ptr<Shape> bunny;
shared_ptr<Shape> teapot;
shared_ptr<Shape> sphere;
shared_ptr<Shape> revolution;
shared_ptr<Shape> plane;
shared_ptr<Shape> frustum;
shared_ptr<Light> sun;
glm::vec4 lightPos(0.0f, 2.0f, 0.0f, 1.0f);

int numLights;
glm::vec3 lights[256];

vector<shared_ptr<Program>> progVec;
vector<shared_ptr<Material>> mtrlVec;
vector<shared_ptr<Thing>> thingVec;
vector<shared_ptr<Light>> lightVec;

Material hudMat;
float fx;
float fy;
float scaleFactor;
glm::vec3 hudLightPos;

bool zoom;

bool shift;


bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt = (mods & GLFW_MOD_ALT) != 0;
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {
		camera->mouseMoved((float)xmouse, (float)ymouse);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "blph_vert.glsl", RESOURCE_DIR + "blph_frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addUniform("MV");
	prog->addUniform("invMV");
	prog->addUniform("camMV");
	prog->addUniform("P");
	prog->addUniform("lightPos");
	prog->addUniform("lightColor");
	prog->addUniform("ke");
	prog->addUniform("kd");
	prog->addUniform("ks");
	prog->addUniform("s");
	prog->addUniform("polar");
	prog->addUniform("t");
	prog->addUniform("lights");
	prog->addUniform("numLights");
	prog->setVerbose(false);

	camera = make_shared<Camera>();
	camera->setInitDistance(20.0f); // FreelookCam's initial Z translation
	
	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();
	
	plane = make_shared<Shape>();
	plane->loadMesh(RESOURCE_DIR + "plane.obj");
	plane->init();

	sphere = make_shared<Shape>();
	
	vector<float> spPosBuf;
	vector<float> spNorBuf;
	vector<float> spTexBuf;
	vector<unsigned int> spIndBuf;
	float height = 1.0;
	float width = 1.0;
	int num_rows = 100;
	int num_cols = 100;

	for (int i = 0; i <= num_rows; i++) {
		float v = i / (float)num_rows * height;
		float phi = v * M_PI;
		for (int j = 0; j <= num_cols; j++) {
			float u = j / (float)num_cols * width;
			float theta = u * 2 * M_PI;

			float x = sin(theta) * sin(phi) * width / 2;
			float y = cos(theta) * height / 2;
			float z = sin(theta) * cos(phi) * width / 2;

			spPosBuf.push_back(x);
			spPosBuf.push_back(y);
			spPosBuf.push_back(z);

			spNorBuf.push_back(sin(theta) * sin(phi));
			spNorBuf.push_back(cos(theta));
			spNorBuf.push_back(sin(theta) * cos(phi));
		}
	}

	for (int i = 0; i < num_cols; i++) {
		for (int j = 0; j < num_rows; j++) {
			int v1 = (num_rows + 1) * i + j;
			int v2 = v1 + 1;
			int v3 = v1 + num_rows + 1;
			int v4 = v2 + num_rows + 1;

			spIndBuf.push_back(v1);
			spIndBuf.push_back(v4);
			spIndBuf.push_back(v2);

			spIndBuf.push_back(v1);
			spIndBuf.push_back(v3);
			spIndBuf.push_back(v4);
		}
	}

	sphere->loadPoints(spPosBuf, spNorBuf, spTexBuf, spIndBuf);
	sphere->init();

	vector<float> rvPosBuf;
	vector<float> rvNorBuf;
	vector<float> rvTexBuf;
	vector<unsigned int> rvIndBuf;
	width = 2.0;

	for (int i = 0; i <= num_cols; i++) {
		float u = i / (float)num_cols;
		float x = u * width - width / 2;
		for (int j = 0; j <= num_rows; j++) {
			float v = j / (float)num_rows;
			float theta = v * 2 * M_PI;

			float c = 0.2;
			float s = 5;
			float h = 2;

			float f = c * (cos(s * x) + h);
			float y = f * cos(theta);
			float z = f * sin(theta);

			glm::vec3 dpdx(
				1.0f,
				-c * s * sin(s * x) * cos(theta),
				-c * s * sin(s * x) * sin(theta)
			);

			glm::vec3 dpdt(
				0.0f,
				-c * (cos(s * x) + h) * sin(theta),
				 c * (cos(s * x) + h) * cos(theta)
			);

			glm::vec3 n = glm::normalize(glm::cross(dpdx, dpdt));

			rvPosBuf.push_back(y);
			rvPosBuf.push_back(x);
			rvPosBuf.push_back(z);

			rvNorBuf.push_back(n.y);
			rvNorBuf.push_back(n.x);
			rvNorBuf.push_back(n.z);
		}
	}

	for (int i = 0; i < num_cols; i++) {
		for (int j = 0; j < num_rows; j++) {
			int v1 = (num_rows + 1) * i + j;
			int v2 = v1 + 1;
			int v3 = v1 + num_rows + 1;
			int v4 = v2 + num_rows + 1;

			rvIndBuf.push_back(v1);
			rvIndBuf.push_back(v4);
			rvIndBuf.push_back(v2);

			rvIndBuf.push_back(v1);
			rvIndBuf.push_back(v3);
			rvIndBuf.push_back(v4);
		}
	}

	revolution = make_shared<Shape>();
	revolution->loadPoints(rvPosBuf, rvNorBuf, rvTexBuf, rvIndBuf);
	revolution->init();


	GLSL::checkError(GET_FILE_LINE);

	float spacing = 2.0;
	int rowCount = 10;
	int colCount = 10;
	// create things
	for (int i = 0; i < rowCount * colCount; i++)
	{
		shared_ptr<Thing> thing;
		Shape* shape;
		int num = (int)rand() % 4;

		switch (num) {
			case Thing::BUNNY:
				thing = make_shared<Thing>(bunny.get(), Thing::BUNNY);
				break;
			case Thing::TEAPOT:
				thing = make_shared<Thing>(teapot.get(), Thing::TEAPOT);
				break;
			case Thing::SPHERE:
				thing = make_shared<Thing>(sphere.get(), Thing::SPHERE);
				break;
			case Thing::REVOLUTION:
				thing = make_shared<Thing>(revolution.get(), Thing::REVOLUTION);
				break;
		}

		int row = i / colCount;
		int col = i % colCount;

		thing->initPos = glm::vec3(
			(row - (rowCount / 2.0)) * spacing,
			0.0,
			(col - (colCount / 2.0)) * spacing
		);

		thingVec.push_back(thing);
	}


	numLights = 50;
	float cpx = (rowCount) * spacing;
	float cpz = (colCount) * spacing;
	float cc = 0.25;
	for (int i = 0; i < numLights; i++) {
		auto l = make_shared<Light>();
		l->lightPos = glm::vec3(cpx * (randf() - 0.5), 0.5, cpz * (randf() - 0.5));
		l->initPos = l->lightPos;
		l->color = glm::vec3((1 - 2 * cc) * randf() + cc, (1 - 2 * cc) * randf() + cc, (1 - 2 * cc) * randf() + cc);
		l->tOff = randf() * 2 * M_PI;

		lights[2 * i] = l->lightPos;
		lights[2 * i + 1] = l->color;

		lightVec.push_back(l);
	}
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width/(float)height;
	camera->setAspect(aspect);
	
	double t = glfwGetTime();
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	prog->bind();

	glViewport(0, 0, width, height);

	glm::mat4 invMV;

	glUniform1f(prog->getUniform("t"), 0.0f);

	// draw scene
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
		camera->applyViewMatrix(MV);
		glm::mat4 camMV = MV->topMatrix();

		glUniformMatrix4fv(prog->getUniform("camMV"), 1, GL_FALSE, glm::value_ptr(camMV));

		MV->translate(0.0f, -1.0f, 0.0f);
	
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

		MV->pushMatrix();
			glUniform3f(prog->getUniform("kd"), 0.0f, 0.0f, 0.0f);
			glUniform3f(prog->getUniform("ks"), 0.0f, 0.0f, 0.0f);
			glUniform1f(prog->getUniform("s"), 0.0f);

			for (int i = 0; i < lightVec.size(); i++) {
				shared_ptr<Light> l = lightVec.at(i);
				l->update(t);

				lights[2 * i] = l->lightPos;

				MV->pushMatrix();
				MV->translate(l->lightPos);
				MV->scale(0.1f, 0.1f, 0.1f);
				invMV = glm::transpose(glm::inverse(MV->topMatrix()));

				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("invMV"), 1, GL_FALSE, glm::value_ptr(invMV));

				glUniform3f(prog->getUniform("ke"), l->color.x, l->color.y, l->color.z);
				sphere->draw(prog);
				MV->popMatrix();
			}

			glUniform3fv(prog->getUniform("lights"), numLights * 2, value_ptr(lights[0]));
			glUniform1i(prog->getUniform("numLights"), numLights);

			for (shared_ptr<Thing> th : thingVec) {
				MV->pushMatrix();
					MV->translate(th->initPos);
					MV->scale(th->initScale);
					MV->translate(glm::vec3(0.0f, -th->shape->miny, 0.0f));
					MV->rotate(th->initRotY, 0.0, 1.0, 0.0);
					th->update(MV, t);

					invMV = glm::transpose(glm::inverse(MV->topMatrix()));

					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					glUniformMatrix4fv(prog->getUniform("invMV"), 1, GL_FALSE, glm::value_ptr(invMV));

					glUniform3f(prog->getUniform("ke"), th->material.ke.x, th->material.ke.y, th->material.ke.z);
					glUniform3f(prog->getUniform("kd"), th->material.kd.x, th->material.kd.y, th->material.kd.z);
					glUniform3f(prog->getUniform("ks"), th->material.ks.x, th->material.ks.y, th->material.ks.z);
					glUniform1f(prog->getUniform("s"), th->material.s);

					th->draw(prog);
				MV->popMatrix();
			}

			MV->pushMatrix();
				invMV = glm::transpose(glm::inverse(MV->topMatrix()));

				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				glUniformMatrix4fv(prog->getUniform("invMV"), 1, GL_FALSE, glm::value_ptr(invMV));

				glUniform3f(prog->getUniform("ke"), 0.0f, 0.0f, 0.0f);
				glUniform3f(prog->getUniform("kd"), 0.5f, 0.5f, 0.5f);
				glUniform3f(prog->getUniform("ks"), 1.0f, 1.0f, 1.0f);
				glUniform1f(prog->getUniform("s"), 10.0f);

				plane->draw(prog);
			MV->popMatrix();
		MV->popMatrix();
	MV->popMatrix();
	P->popMatrix();

	prog->unbind();
	
	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
