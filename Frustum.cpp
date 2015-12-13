#include "Frustum.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// ============================================================ //
// Frustum::Plane

Frustum::Plane::Plane()
{
	this->A = 0.0f;
	this->B = 0.0f;
	this->C = 0.0f;
	this->D = 0.0f;
}

Frustum::Plane::Plane(float a, float b, float c, float d)
{
	this->A = a;
	this->B = b;
	this->C = c;
	this->D = d;
}

void Frustum::Plane::set(float a, float b, float c, float d)
{
	this->A = a;
	this->B = b;
	this->C = c;
	this->D = d;
}

void Frustum::Plane::normalize()
{
	this->mag = std::sqrtf(A*A + B*B + C*C);
	this->A /= this->mag;
	this->B /= this->mag;
	this->C /= this->mag;
	this->D /= this->mag;

	glm::vec4 v;
	float m = v.length();
	v /= m;
}

float Frustum::Plane::getDistanceToPoint(float x, float y, float z)
{
	return A*x + B*y + C*z + D;
}

float Frustum::Plane::getDistanceToPoint(glm::vec3 & v)
{
	return A*v.x + B*v.y + C*v.z + D;
}


// ============================================================ //
// Frustum

Frustum::Frustum()
{
}


Frustum::~Frustum()
{
}

void Frustum::update(const glm::mat4 & mvp)
{
	const float *f_mvp = glm::value_ptr(mvp);
	// Calculate the clip planes
	// -- Right side Plane
	m_planes[PLANE_RIGHT].set(f_mvp[3] - f_mvp[0], f_mvp[7] - f_mvp[4], f_mvp[11] - f_mvp[8], f_mvp[15] - f_mvp[12]);
	m_planes[PLANE_RIGHT].normalize();
	// -- Left side Plane
	m_planes[PLANE_LEFT].set(f_mvp[3] + f_mvp[0], f_mvp[7] + f_mvp[4], f_mvp[11] + f_mvp[8], m_planes[PLANE_LEFT].D = f_mvp[15] + f_mvp[12]);
	m_planes[PLANE_LEFT].normalize();
	// -- Bottom side Plane
	m_planes[PLANE_BOTTOM].set(f_mvp[3] + f_mvp[1], f_mvp[7] + f_mvp[5], f_mvp[11] + f_mvp[9], f_mvp[15] + f_mvp[13]);
	m_planes[PLANE_BOTTOM].normalize();
	// -- Top side Plane
	m_planes[PLANE_TOP].set(f_mvp[3] - f_mvp[1], f_mvp[7] - f_mvp[5], f_mvp[11] - f_mvp[9], f_mvp[15] - f_mvp[13]);
	m_planes[PLANE_TOP].normalize();
	// -- Far side Plane
	m_planes[PLANE_FAR].set(f_mvp[3] - f_mvp[2], f_mvp[7] - f_mvp[6], f_mvp[11] - f_mvp[10], f_mvp[15] - f_mvp[14]);
	m_planes[PLANE_FAR].normalize();
	// -- Near side Plane
	m_planes[PLANE_NEAR].set(f_mvp[3] + f_mvp[2], f_mvp[7] + f_mvp[6], f_mvp[11] + f_mvp[10], f_mvp[15] + f_mvp[14]);
	m_planes[PLANE_NEAR].normalize();
}

float Frustum::checkSphere(glm::vec3 pos, float radius)
{
	/*
	* Check if the sphere is within the scene Frustum
	* return: distance to the NEAR clip plane of the nearest point of the sphere's radius
	*/
	float d;

	for (int p = 0; p < 6; p++)
	{
		d = m_planes[p].getDistanceToPoint(pos.x, pos.y, pos.z);
		if (d <= -radius) return 0.0f;
	}

	return d + radius;
}


bool Frustum::checkBox(glm::vec3 pos, glm::vec3 boxMin, glm::vec3 boxMax)
{
	float d = 0.0f;

	for (int p = 0; p < 6; p++)
	{
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMin.x, pos.y + boxMin.y, pos.z + boxMin.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMax.x, pos.y + boxMin.y, pos.z + boxMin.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMin.x, pos.y + boxMax.y, pos.z + boxMin.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMax.x, pos.y + boxMax.y, pos.z + boxMin.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMin.x, pos.y + boxMin.y, pos.z + boxMax.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMax.x, pos.y + boxMin.y, pos.z + boxMax.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMin.x, pos.y + boxMax.y, pos.z + boxMax.z)) > 0) continue;
		if ((d = m_planes[p].getDistanceToPoint(pos.x + boxMax.x, pos.y + boxMax.y, pos.z + boxMax.z)) > 0) continue;

		return false;
	}
	return true;
}
