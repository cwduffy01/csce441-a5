#include "Thing.h"
#include "helpers.h"
#include <glm/glm.hpp>
#include <cmath> 
//float randf() { return rand() / (float)RAND_MAX; }

using namespace std;

Thing::Thing(Shape* s) : shape(s) {
	this->initPos = glm::vec3(0.0f, 0.0f, 0.0f);

	this->material.ke = glm::vec3( 0.0f, 0.0f, 0.0f );
	this->material.kd = glm::vec3( randf(), randf(), randf() );
	this->material.ks = glm::vec3( 1.0f, 1.0f, 1.0f );
	this->material.s = 10.0;

	initRotY = randf() * 2 * 3.14;

	initScale = 0.5 * randf() + 0.5;
	scaleFactor = 0.1 * randf() + 0.1;
	timeShift = randf() * 2 * 3.14;
}

glm::vec3 Thing::getScale(float time) {
	float factor = initScale + scaleFactor * sin(1.0*(time + timeShift));
	return glm::vec3(factor, factor, factor);
}