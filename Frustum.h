#pragma once

#include <cmath>
#include "glm/fwd.hpp"


class Frustum
{
public:

	class Plane
	{
	public:
		float A, B, C, D, mag;

		Plane();
		Plane(float a, float b, float c, float d);

		void set(float a, float b, float c, float d);
		void normalize();
		float getDistanceToPoint(float x, float y, float z);
		float getDistanceToPoint(glm::vec3 & v);
	};

	Frustum();
	~Frustum();

	void update(const glm::mat4 & mvp);
	float checkSphere(glm::vec3 pos, float radius);
	bool checkBox(glm::vec3 pos, glm::vec3 boxMin, glm::vec3 boxMax);

protected:
	Plane	m_planes[6];

	enum {
		PLANE_RIGHT = 0,
		PLANE_LEFT = 1,
		PLANE_BOTTOM = 2,
		PLANE_TOP = 3,
		PLANE_FAR = 4,
		PLANE_NEAR = 5
	};
};

