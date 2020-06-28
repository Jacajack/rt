#pragma once
#include <glm/glm.hpp>

namespace rt {

/**
	Bidirectional Reflectance Distribution Function abstract base class
*/
class abstract_brdf
{
public:
	virtual glm::vec3 operator()(const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const = 0;
};

/**
	Monochromatic diffuse Phong reflection model
*/
class diffuse_brdf : public abstract_brdf
{
public:
	diffuse_brdf(const glm::vec3 &color) :
		m_color(color)
	{}

	glm::vec3 operator()(const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const override
	{
		return m_color * glm::clamp(glm::dot(N, L), 0.f, 1.f);
	}

private:
	glm::vec3 m_color;
};

}