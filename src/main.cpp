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
shared_ptr<Shape> cube;
shared_ptr<Shape> frustum;
shared_ptr<Shape> square;
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

shared_ptr<Program> progPass1;
shared_ptr<Program> progPass2;

GLuint framebufferID;
GLuint posTexture;
GLuint norTexture;
GLuint keTexture;
GLuint kdTexture;
int textureWidth = 640;
int textureHeight = 480;

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

	progPass1 = make_shared<Program>();
	progPass1->setShaderNames(RESOURCE_DIR + "pass1_vert.glsl", RESOURCE_DIR + "pass1_frag.glsl");
	progPass1->setVerbose(true);
	progPass1->init();
	progPass1->addAttribute("aPos");
	progPass1->addAttribute("aNor");
	progPass1->addUniform("P");
	progPass1->addUniform("MV");
	progPass1->addUniform("invMV");
	progPass1->addUniform("ke");
	progPass1->addUniform("kd");
	progPass1->setVerbose(false);

	progPass2 = make_shared<Program>();
	progPass2->setShaderNames(RESOURCE_DIR + "pass2_vert.glsl", RESOURCE_DIR + "pass2_frag.glsl");
	progPass2->setVerbose(true);
	progPass2->init();
	progPass2->addAttribute("aPos");
	progPass2->addUniform("P");
	progPass2->addUniform("MV");
	progPass2->addUniform("posTexture");
	progPass2->addUniform("norTexture");
	progPass2->addUniform("keTexture");
	progPass2->addUniform("kdTexture");
	progPass2->addUniform("windowSize");
	progPass2->addUniform("lights");
	progPass2->addUniform("numLights");
	progPass2->addUniform("camMV");
	progPass2->addUniform("blur");
	progPass2->bind();
	glUniform1i(progPass2->getUniform("posTexture"), 0);
	glUniform1i(progPass2->getUniform("norTexture"), 1);
	glUniform1i(progPass2->getUniform("keTexture"), 2);
	glUniform1i(progPass2->getUniform("kdTexture"), 3);
	progPass2->unbind();
	progPass2->setVerbose(false);

	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	glGenTextures(1, &posTexture);
	glBindTexture(GL_TEXTURE_2D, posTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTexture, 0);

	glGenTextures(1, &norTexture);
	glBindTexture(GL_TEXTURE_2D, norTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, norTexture, 0);

	glGenTextures(1, &keTexture);
	glBindTexture(GL_TEXTURE_2D, keTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, keTexture, 0);

	glGenTextures(1, &kdTexture);
	glBindTexture(GL_TEXTURE_2D, kdTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, kdTexture, 0);

	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cerr << "Framebuffer is not ok" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

	cube = make_shared<Shape>();
	cube->loadMesh(RESOURCE_DIR + "cube.obj");
	cube->init();

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

	vector<float> sqPosBuf;
	vector<float> sqNorBuf;
	vector<float> sqTexBuf;
	vector<unsigned int> sqIndBuf;

	sqPosBuf.push_back(-0.5f);
	sqPosBuf.push_back(-0.5f);
	sqPosBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(1.0f);

	sqPosBuf.push_back(-0.5f);
	sqPosBuf.push_back(0.5f);
	sqPosBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(1.0f);

	sqPosBuf.push_back(0.5f);
	sqPosBuf.push_back(-0.5f);
	sqPosBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(1.0f);

	sqPosBuf.push_back(0.5f);
	sqPosBuf.push_back(0.5f);
	sqPosBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(0.0f);
	sqNorBuf.push_back(1.0f);

	sqIndBuf.push_back(0);
	sqIndBuf.push_back(1);
	sqIndBuf.push_back(3);

	sqIndBuf.push_back(0);
	sqIndBuf.push_back(2);
	sqIndBuf.push_back(3);

	square = make_shared<Shape>();
	square->loadPoints(sqPosBuf, sqNorBuf, sqTexBuf, sqIndBuf);
	square->init();

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
	glm::mat4 invMV;


	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glViewport(0, 0, textureWidth, textureHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	progPass1->bind();
		P->pushMatrix();
		camera->applyProjectionMatrix(P);
			MV->pushMatrix();
			camera->applyViewMatrix(MV);
				glm::mat4 camMV = MV->topMatrix();
				MV->translate(0.0f, -1.0f, 0.0f);
				glUniformMatrix4fv(progPass1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
				glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

				MV->pushMatrix();
					// draw lights
					glUniform3f(progPass1->getUniform("kd"), 0.0f, 0.0f, 0.0f);
					for (int i = 0; i < lightVec.size(); i++) {
						shared_ptr<Light> l = lightVec.at(i);
						l->update(t);

						lights[2 * i] = l->lightPos;

						MV->pushMatrix();
							MV->translate(l->lightPos);
							MV->scale(0.1f, 0.1f, 0.1f);
							invMV = glm::transpose(glm::inverse(MV->topMatrix()));

							glUniformMatrix4fv(progPass1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
							glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
							glUniformMatrix4fv(progPass1->getUniform("invMV"), 1, GL_FALSE, glm::value_ptr(invMV));

							glUniform3f(progPass1->getUniform("ke"), l->color.x, l->color.y, l->color.z);
							sphere->draw(progPass1);
						MV->popMatrix();
					}

					// draw things
					for (shared_ptr<Thing> th : thingVec) {
						MV->pushMatrix();
							MV->translate(th->initPos);
							MV->scale(th->initScale);
							MV->translate(glm::vec3(0.0f, -th->shape->miny, 0.0f));
							MV->rotate(th->initRotY, 0.0, 1.0, 0.0);
							th->update(MV, t);

							invMV = glm::transpose(glm::inverse(MV->topMatrix()));

							glUniformMatrix4fv(progPass1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
							glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
							glUniformMatrix4fv(progPass1->getUniform("invMV"), 1, GL_FALSE, glm::value_ptr(invMV));

							glUniform3f(progPass1->getUniform("ke"), th->material.ke.x, th->material.ke.y, th->material.ke.z);
							glUniform3f(progPass1->getUniform("kd"), th->material.kd.x, th->material.kd.y, th->material.kd.z);

							th->draw(progPass1);
						MV->popMatrix();
					}

					// draw floor
					MV->pushMatrix();
						invMV = glm::transpose(glm::inverse(MV->topMatrix()));

						glUniformMatrix4fv(progPass1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
						glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
						glUniformMatrix4fv(progPass1->getUniform("invMV"), 1, GL_FALSE, glm::value_ptr(invMV));

						glUniform3f(progPass1->getUniform("ke"), 0.0f, 0.0f, 0.0f);
						glUniform3f(progPass1->getUniform("kd"), 1.0f, 1.0f, 1.0f);

						plane->draw(progPass1);
					MV->popMatrix();
				MV->popMatrix();
			MV->popMatrix();
		P->popMatrix();
	progPass1->unbind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, textureWidth, textureHeight);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	progPass2->bind();
		if (keyToggles[(unsigned)'b']) {
			glUniform1i(progPass2->getUniform("blur"), true);
		}
		else {
			glUniform1i(progPass2->getUniform("blur"), false);
		}

		P->pushMatrix();
		camera->applyProjectionMatrix(P);
			MV->pushMatrix();
			//camera->applyViewMatrix(MV);
				MV->translate(0.0, 0.0, -5.0);
				float sc = 7.0;
				MV->scale(sc, sc, sc);

				glUniformMatrix4fv(progPass2->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
				glUniformMatrix4fv(progPass2->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				glm::vec2 wSize((float)width, float(height));
				glUniform2f(progPass2->getUniform("windowSize"), (float)width, (float)height);
				glUniformMatrix4fv(progPass2->getUniform("camMV"), 1, GL_FALSE, glm::value_ptr(camMV));
				glUniform3fv(progPass2->getUniform("lights"), numLights * 2, value_ptr(lights[0]));
				glUniform1i(progPass2->getUniform("numLights"), numLights);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, posTexture);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, norTexture);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, keTexture);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, kdTexture);

				
				square->draw(progPass2);

				glActiveTexture(GL_TEXTURE0);
			MV->popMatrix();
		P->popMatrix();
	progPass2->unbind();
	
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
