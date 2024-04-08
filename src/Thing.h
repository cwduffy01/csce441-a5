#ifndef THING_H
#define THING_H

#include "Shape.h"
#include "Material.h"
#include "MatrixStack.h"

class Thing {
public:
	enum shape_type {
		BUNNY,
		TEAPOT,
		SPHERE,
		REVOLUTION
	};

	Shape* shape;
	Material material;
	shape_type type;

	glm::vec3 initPos;
	glm::vec3 initScale;
	float initRotY;

	Thing(Shape* shape, shape_type type);

	glm::vec3 getScale(float time);
	void update(shared_ptr<MatrixStack> MV, double t);

private:
	float scaleFactor;
	float timeShift;
};

#endif // !THING_H
