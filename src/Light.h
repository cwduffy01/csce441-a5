#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

using namespace std;

class Light {
public:
	Light();
	Light(glm::vec3 lightPos, glm::vec3 color);

	glm::vec3 lightPos;
	glm::vec3 color;

private:

};

#endif