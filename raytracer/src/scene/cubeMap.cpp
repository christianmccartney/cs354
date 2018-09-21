#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {

	// YOUR CODE HERE
	// FIXME: Implement Cube Map here

	glm::dvec3 normals[6] = {glm::dvec3(1, 0, 0),
				glm::dvec3(-1, 0, 0),
				glm::dvec3(0, 1, 0),
				glm::dvec3(0, -1, 0),
				glm::dvec3(0, 0, 1),
				glm::dvec3(0, 0, -1)};
	int i = -1;
	double min = 0;
	for (int j = 0; j < 6; j++) {
		double t = 1.0 / glm::dot(normals[j], r.d);
        	if (t > 0 && (t < min || j == -1)) {
			i = j;
			min = t;
		}
	}

	glm::dvec3 p = r.d * min;
	glm::dvec3 proj = p - glm::dot(p, normals[i]) * normals[i];
	glm::dvec2 proj2d(0, 0);

	if (i < 2) {
		proj2d = glm::dvec2(proj.z, proj.y);
	}
	if (i == 2 || i == 3) {
		 proj2d = glm::dvec2(proj.x, proj.z);
	}
	if (i == 4 || i == 5) {
		proj2d = glm::dvec2(proj.x, proj.y);
	}

	glm::dvec2 coord = (proj2d + glm::dvec2(1, 1)) / 2.0;

	return tMap[i]->getMappedValue(coord);
}
