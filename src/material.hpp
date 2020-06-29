#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"

namespace rt {

/**
	Materials provide BSDF based on ray_intersection objects.
	In other words, materials provide scattering information for intersection points.

	Additionally, materials determine what types can/should be cast from the intersection point.

	In the future, materials will also provide functions for importance BSDF sampling.
*/
class material
{
public:
	material(bool emissive, bool transmissive) :
		m_is_emissive(emissive),
		m_is_transmissive(transmissive)
	{}

	material(const material &) = default;
	material(material &&) = default;

	material &operator=(const material &) = default;
	material &operator=(material &&) = default;

	virtual ~material() = default;

	virtual glm::vec3 brdf(const ray_hit &hit, const glm::vec3 &light, const glm::vec3 &view, const glm::vec3 &normal) const = 0;
	
	bool is_emissive() const
	{
		return m_is_emissive;
	}

	bool is_trasmissive() const
	{
		return m_is_transmissive;
	}

private:
	bool m_is_emissive;
	bool m_is_transmissive;
};

/**
	A test Phong-like material
*/
class test_material : public material
{
public:
	test_material(const glm::vec3 &color) :
		material(false, false),
		m_color(color)
	{}

	glm::vec3 brdf(const ray_hit &hit, const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const override
	{
		return m_color * glm::clamp(glm::dot(N, L), 0.f, 1.f);
	}

private:
	glm::vec3 m_color;
};

}