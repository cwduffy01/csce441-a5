#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "FreelookCam.h"
#include "MatrixStack.h"

FreelookCam::FreelookCam() :
	aspect(1.0f),
	fovy((float)(45.0*M_PI/180.0)),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 0.0f, -5.0f),
	rfactor(0.005f),
	tfactor(0.001f),
	sfactor(0.005f),
	rthresh(60 * M_PI / 180.0),
	eye(0.0f, 0.0f, 0.0f),
	target(0.0f, 0.0f, -1.0f),
	up(0.0f, 1.0f, 0.0f),
	forward(target),
	udir(0.0f),
	vdir(0.0f),
	pitch(0.0f),
	yaw(0.0f),
	z_adjust(0.0f)
{
	mousePrev = glm::vec2(0.0f, 0.0f);
}

FreelookCam::~FreelookCam()
{
}



void FreelookCam::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	//if(shift) {
	//	state = FreelookCam::TRANSLATE;
	//} else if(ctrl) {
	//	state = FreelookCam::SCALE;
	//} else {
	//	state = FreelookCam::ROTATE;
	//}
}

void FreelookCam::mouseMoved(float x, float y)
{
	//if (mousePrev.x == 0 && )
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;

	yaw -= rfactor * dv.x;
	pitch -= rfactor * dv.y;
	//std::cout << yaw << " " << pitch << std::endl;
	if (pitch < -rthresh) { pitch = -rthresh; }
	else if (pitch > rthresh) { pitch = rthresh; }

	//if (pitch >= -rthresh && pitch <= rthresh) { pitch -= rfactor * dv.y; }
	//switch(state) {
	//	case FreelookCam::ROTATE:
	//		rotations += rfactor * dv;
	//		break;
	//	case FreelookCam::TRANSLATE:
	//		translations.x -= translations.z * tfactor * dv.x;
	//		translations.y += translations.z * tfactor * dv.y;
	//		break;
	//	case FreelookCam::SCALE:
	//		translations.z *= (1.0f - sfactor * dv.y);
	//		break;
	//}
	mousePrev = mouseCurr;
}

void FreelookCam::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const
{
	// Modify provided MatrixStack
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void FreelookCam::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const
{
	//MV->translate(translations);
	//MV->rotate(rotations.y, glm::vec3(1.0f, 0.0f, 0.0f));
	//MV->rotate(rotations.x, glm::vec3(0.0f, 1.0f, 0.0f));

	MV->multMatrix(glm::lookAt(this->eye, this->target, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void FreelookCam::startDirection(FreelookCam::direction dir) {
	switch (dir) {
		case FreelookCam::FORWARDS:
			this->udir += 1.0;
			break;
		case FreelookCam::BACKWARDS:
			this->udir += -1.0;
			break;
		case FreelookCam::LEFT:
			this->vdir += 1.0;
			break;
		case FreelookCam::RIGHT:
			this->vdir += -1.0;
			break;
	}
}

void FreelookCam::stopDirection(FreelookCam::direction dir) {
	switch (dir) {
		case FreelookCam::FORWARDS:
			this->udir -= 1.0;
			break;
		case FreelookCam::BACKWARDS:
			this->udir -= -1.0;
			break;
		case FreelookCam::LEFT:
			this->vdir -= 1.0;
			break;
		case FreelookCam::RIGHT:
			this->vdir -= -1.0;
			break;
	}
}

void FreelookCam::walk() {
	float factor = 0.1;

	forward.x = sin(yaw);
	//forward.y = sin(pitch);
	forward.z = cos(yaw);
	forward = glm::normalize(forward);

	target = eye + forward;
	target.y = sin(pitch);


	//glm::vec3 dirVec = glm::normalize(this->target - this->eye);
	glm::vec3 perp = glm::normalize(glm::cross(up, forward));
	glm::vec3 update = factor * (udir * forward + vdir * perp);

	this->target += update;
	this->eye += update;

	fovy += z_adjust;
	float min_fovy = (float)(4.0*M_PI/180.0);
	float max_fovy = (float)(114.0*M_PI/180.0);
	if (fovy > max_fovy) { fovy = max_fovy; }
	if (fovy < min_fovy) { fovy = min_fovy; }
	//std::cout << fovy << std::endl;
}

float FreelookCam::getFOV() { return fovy; }

void FreelookCam::startZoom(FreelookCam::zoom z) {
	float val = 0.01;
	if (z == FreelookCam::ZOOM_IN)  { z_adjust =  val; }
	if (z == FreelookCam::ZOOM_OUT) { z_adjust = -val; }
}

void FreelookCam::stopZoom() {
	z_adjust = 0.0;
}