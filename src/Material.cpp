#include "Material.h"

Material::Material() : ke(0.0f, 0.0f, 0.0f), kd(0.0f, 0.0f, 0.0f), ks(0.0f, 0.0f, 0.0f), s(0.0) {}

Material::Material(glm::vec3 ke, glm::vec3 kd, glm::vec3 ks, float s) {
	this->ke = ke;
	this->kd = kd;
	this->ks = ks;
	this->s = s;
}
