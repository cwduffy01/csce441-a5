#pragma  once
#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class MatrixStack;

class FreelookCam
{
public:
	enum {
		ROTATE = 0,
		TRANSLATE,
		SCALE
	};

	enum direction {
		FORWARDS,
		BACKWARDS,
		LEFT,
		RIGHT
	};

	enum zoom {
		ZOOM_IN,
		ZOOM_OUT
	};
	
	FreelookCam();
	virtual ~FreelookCam();
	void setInitDistance(float z) { translations.z = -std::abs(z); }
	void setAspect(float a) { aspect = a; };
	void setRotationFactor(float f) { rfactor = f; };
	void setTranslationFactor(float f) { tfactor = f; };
	void setScaleFactor(float f) { sfactor = f; };
	void mouseClicked(float x, float y, bool shift, bool ctrl, bool alt);
	void mouseMoved(float x, float y);
	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
	void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;
	void walk();
	void startDirection(direction dir);
	void stopDirection(direction dir);
	void startZoom(zoom z);
	void stopZoom();
	void adjustZoom(bool decrease=false);
	float getFOV();

	glm::vec3 eye;
	glm::vec3 target;
	const glm::vec3 up;
	glm::vec3 forward;
	
private:
	float aspect;
	float fovy;
	float znear;
	float zfar;
	glm::vec2 rotations;
	glm::vec3 translations;
	glm::vec2 mousePrev;
	int state;
	float rfactor;
	float tfactor;
	float sfactor;
	float rthresh;

	float udir;
	float vdir;
	float yaw;
	float pitch;
	float z_adjust;
};

#endif
