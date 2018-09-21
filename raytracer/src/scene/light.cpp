#include <cmath>
#include "ray.h"
#include "light.h"
#include <glm/glm.hpp>
#include <iostream>

using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
        glm::dvec3 shadowDirection = getDirection(p);
	glm::normalize(shadowDirection);

        isect i;

        ray shadowRay(p, shadowDirection, r.getPixel(), r.ctr, r.getAtten(), ray::SHADOW);

        if (this->getScene()->intersect(shadowRay, i)) {
		glm::dvec3 q = shadowRay.at(i.t);
		glm::dvec3 kt = i.material->kt(i);
		glm::dvec3 intensity = shadowAttenuation(r, q);

		return intensity * kt;
        }

        return color;
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const {
	glm::dvec3 direction = position - P;
	double distance = glm::length(direction);
	return min(1.0, 1/(constantTerm + linearTerm*distance + quadraticTerm*distance*distance));
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const {

	glm::dvec3 shadowDirection = (position-p);
	glm::normalize(shadowDirection);

	isect i;
	ray shadowRay(p, shadowDirection, r.getPixel(), r.ctr, r.getAtten(), ray::SHADOW);

	if (this->getScene()->intersect(shadowRay, i)) {
		double lightDistance = glm::length(position - p);
		glm::dvec3 q = shadowRay.at(i.t);
		double distance = glm::length(q - p);

		if (distance < lightDistance) {
			const Material& m = i.getMaterial();
			return m.kt(i) * color;
		}
	}
	return color;
}

