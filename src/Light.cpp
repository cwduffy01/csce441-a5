#include "Light.h"

Light::Light() : lightPos(0.0f, 0.0f, 0.0f), color(0.0f, 0.0f, 0.0f) {}

Light::Light(glm::vec3 lightPos, glm::vec3 color) {
	this->color = color;
	this->lightPos = lightPos;
}