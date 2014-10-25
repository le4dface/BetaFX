
#include <iostream>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../pipeline/Pipeline.h"
#include "Camera.h"

namespace std {

Camera::Camera():
		focus {0, 0, 0},
		cam_angle {1, 0, 0, 0},
		cam_angle_d {1, 0, 0, 0},
		camera_properties { GL_SHADER_STORAGE_BUFFER }
{
	cam_aspect = 1.0;
	viewzoom = 40.0;
	windowwidth = windowheight = 1;

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

}

Camera::~Camera() {}

void Camera::update( float tick ) {
	cam_angle = cam_angle_d * cam_angle;
//	printf("camera angle: %f %f %f %f\n", cam_angle_d.w, cam_angle_d.x, cam_angle_d.y, cam_angle_d.z);
	cam_angle_d = glm::quat(); //glm::slerp( , cam_angle_d, ( 1 - tick * 10 ) );

	float x = focus.x, y = focus.y, z = focus.z-viewzoom;
	camera_properties.data->P = glm::perspective(45.0f, cam_aspect, 1.0f, 100.0f);
	camera_properties.data->V = glm::mat4_cast(cam_angle) * glm::translate( glm::mat4(1.0f), glm::vec3(x, y, z) );
//	camera_properties.data->V = glm::mat4_cast(cam_angle);
	camera_properties.data->M = glm::mat4(1.0f);
	camera_properties.update();
}

void Camera::resize(int x, int y) {
	windowwidth = x;
	windowheight = y;
	cam_aspect = (double) x / (double) y;
}

void Camera::printAngle() {
	cout << "camera quat = " << cam_angle.w << ", " << cam_angle.x << ", " <<  cam_angle.y << ", " <<  cam_angle.z << endl;
	cout << "camera zoom = " << viewzoom << endl;
}

void Camera::zoom(float z) {
	viewzoom *= z;
}

void Camera::rotate(glm::quat q) {
//	cam_angle_d = q * cam_angle_d;
	cam_angle = q;
}

void Camera::translate(glm::vec3 t) {
	focus += t;
}

glm::quat Camera::cameraAngle() {
	return cam_angle;
}

glm::vec3 Camera::project(const glm::vec3 &v) {
	return glm::project( v, camera_properties.data->V, camera_properties.data->P, glm::vec4(0, 0, windowwidth, windowheight) );
}

glm::vec3 Camera::unProject(int x, int y) {
	GLdouble point[3];
	// TODO
	return glm::vec3(point[0], point[1], point[2]);
}

glm::mat4 Camera::viewMatrix() {
	return camera_properties.data->V;
}

glm::mat4 Camera::projectionMatrix() {
	return camera_properties.data->P;
}

Buffer<CameraProperties> *Camera::properties() {
	return &camera_properties;
}

} /* namespace std */
