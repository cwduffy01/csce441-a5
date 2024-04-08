#ifndef THING_H
#define THING_H

#include "Shape.h"
#include "Material.h"

class Thing {
public:
	Shape* shape;
	Material material;

	glm::vec3 initPos;
	float initRotY;

	Thing(Shape* shape);

	glm::vec3 getScale(float time);

private:
	float initScale;
	float scaleFactor;
	float timeShift;
};

#endif // !THING_H
