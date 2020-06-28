#pragma once
#include <glm/glm.hpp>

namespace rt {

/**
	Abstract material class
*/
class material
{
public:
	virtual glm::vec3 brdf(const glm::vec3 &incident, const glm::vec3 &view, const glm::vec3 &normal) const = 0;
};

}