#include "Thing.h"
#include "helpers.h"
#include "Program.h"
#include <glm/glm.hpp>
#include <cmath> 
#include <math.h>
#include <GL/glew.h>

using namespace std;

Thing::Thing(Shape* s, Thing::shape_type t) : shape(s), type(t) {
	this->initPos = glm::vec3(0.0f, 0.0f, 0.0f);

	this->material.ke = glm::vec3( 0.0f, 0.0f, 0.0f );
	this->material.kd = glm::vec3( randf(), randf(), randf() );
	this->material.ks = glm::vec3( 1.0f, 1.0f, 1.0f );
	this->material.s = 10.0;

	initRotY = randf() * 2 * 3.14;

	float sc = 0.5 * randf() + 0.5;
	initScale = glm::vec3(sc, sc, sc);
	scaleFactor = 0.1 * randf() + 0.1;
	timeShift = randf() * 2 * 3.14;
}

void Thing::update(shared_ptr<MatrixStack> MV, double t) {
	float h = 1;
	float s = 0.25;
	float r = 2.0;

	switch(type) {
		case Thing::BUNNY:
			MV->rotate(-1.5 * (t + timeShift), 0.0f, 1.0f, 0.0f);
			break;
		case Thing::TEAPOT:
			MV->rotate(1.57, 0.0f, 1.0f, 0.0f);
			glm::mat4 S(1.0f);
			S[1][0] = 0.7f * cos(2 * (t + timeShift));
			MV->multMatrix(S);
			MV->rotate(-1.57, 0.0f, 1.0f, 0.0f);
			break;
		case Thing::SPHERE:
			
			MV->translate(0.0f, (h / 2) * (-1 * cos(r * (t + timeShift)) + 1), 0.0f);
			MV->scale(1 + (s / 2) * (cos(2 * r * (t + timeShift)) - 1), 1.0, 1 + (s / 2) * (cos(2 * r * (t + timeShift)) - 1));
			break;
		default:
			break;
	}
}

void Thing::draw(std::shared_ptr<Program> prog) {
	switch (type) {
	case Thing::BUNNY:
	case Thing::TEAPOT:
	case Thing::SPHERE:
		glUniform1i(prog->getUniform("polar"), 0);
		break;
	case Thing::REVOLUTION:
		glUniform1i(prog->getUniform("polar"), 0);
		break;
	}
	shape->draw(prog);
}
