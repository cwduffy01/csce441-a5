#include "Light.h"

Light::Light() : lightPos(0.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f) {
	this->initPos = lightPos;
}

Light::Light(glm::vec3 lightPos, glm::vec3 color) {
	this->color = color;
	this->lightPos = lightPos;
	this->initPos = lightPos;
}

void Light::update(double t) {
	float r = length(initPos);

	float theta = atan2(initPos.x, initPos.z) + 0.1 * t;

	lightPos = glm::vec3(
		r * sin(theta),
		0.5,
		r * cos(theta)
	);
}