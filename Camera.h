#pragma once

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <glm/glm.hpp>
#include <cstdint>

class Camera
{
public:

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 lookat;
	glm::vec3 angle;
};

#endif // __CAMERA_H__
