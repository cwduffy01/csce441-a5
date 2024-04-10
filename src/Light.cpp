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
	float r = pow(pow(initPos.x, 2) + pow(initPos.z, 2), 0.5);

	float theta = atan2(initPos.x, initPos.z) + 0.1 * t;

	lightPos = glm::vec3(
		r * sin(theta),
		initPos.y + 0.2 * sin(t + tOff),
		r * cos(theta)
	);
}