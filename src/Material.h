#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

using namespace std;

class Material {
public:
	Material();
	Material(glm::vec3 ke, glm::vec3 kd, glm::vec3 ks, float s);

	glm::vec3 ke;
	glm::vec3 kd;
	glm::vec3 ks;
	float s;

private:

};

#endif