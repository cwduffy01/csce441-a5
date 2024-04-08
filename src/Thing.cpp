#include "Thing.h"
#include "helpers.h"
#include <glm/glm.hpp>
#include <cmath> 
//float randf() { return rand() / (float)RAND_MAX; }

using namespace std;

Thing::Thing(Shape* s, Thing::shape_type t) : shape(s), type(t) {
	this->initPos = glm::vec3(0.0f, 0.0f, 0.0f);

	this->material.ke = glm::vec3( 0.0f, 0.0f, 0.0f );
	this->material.kd = glm::vec3( randf(), randf(), randf() );
	this->material.ks = glm::vec3( 1.0f, 1.0f, 1.0f );
	this->material.s = 10.0;

	initRotY = randf() * 2 * 3.14;

	float sc = 0.5 * randf() + 0.5;
	//sc = 0.3;
	initScale = glm::vec3(sc, sc, sc);
	scaleFactor = 0.1 * randf() + 0.1;
	timeShift = randf() * 2 * 3.14;
}

//glm::vec3 Thing::getScale(float time) {
//	float factor = initScale + scaleFactor * sin(1.0*(time + timeShift));
//	return glm::vec3(factor, factor, factor);
//}

void Thing::update(shared_ptr<MatrixStack> MV, double t) {
	switch(type) {
		case Thing::BUNNY:
			MV->rotate(-1.5 * t, 0.0f, 1.0f, 0.0f);
			break;
		case Thing::TEAPOT:
			glm::mat4 S(1.0f);
			S[1][0] = 0.7f * cos(2 * t);
			MV->multMatrix(S);
			break;
		default:
			break;
	}
}