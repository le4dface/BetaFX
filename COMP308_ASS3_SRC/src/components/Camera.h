
#ifndef CAMERA_H_
#define CAMERA_H_

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../buffer/Buffer.h"

namespace std {

struct CameraProperties {
	glm::mat4 P;
	glm::mat4 V;
	glm::mat4 M;
};

void getArc(int, int, int, int, float, glm::quat &);
void getUnitCircle(int, int, int, int, glm::quat &);

class Camera {
private:
	/*
	 * point the camera looks at
	 */
	glm::vec3 focus;

	glm::quat cam_angle, cam_angle_d;
	int windowwidth, windowheight;
	float viewzoom, cam_aspect;
	Buffer<CameraProperties> camera_properties;

public:
	Camera();
	virtual ~Camera();

	void update( float );
	void resize(int, int);

	void printAngle();
	void zoom(float);
	void rotate(glm::quat);
	void translate(glm::vec3);

	void keyPressed(unsigned char c);


	glm::quat cameraAngle();
	glm::vec3 project(const glm::vec3 &);
	glm::vec3 unProject(int, int);
	glm::mat4 viewMatrix();
	glm::mat4 projectionMatrix();
	Buffer<CameraProperties> *properties();
};

} /* namespace std */
#endif /* CAMERA_H_ */
