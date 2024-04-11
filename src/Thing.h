#ifndef THING_H
#define THING_H

#include "Shape.h"
#include "Material.h"
#include "MatrixStack.h"
#include <cmath>

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

	void draw(const std::shared_ptr<Program> prog);

	float timeShift;

private:
	float scaleFactor;
};

#endif // !THING_H
