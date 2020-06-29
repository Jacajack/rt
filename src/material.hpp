#pragma once
#include <glm/glm.hpp>
#include "ray.hpp"
#include "scene.hpp"

namespace rt {

// A forward declaration of ray_hit
class ray_hit;

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
	A simple sky material that can be used by the scene
*/
class simple_sky_material : public material
{
public:
	simple_sky_material() :
		material(true, false)
	{}

	glm::vec3 brdf(const ray_hit &hit, const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const override
	{
		return glm::vec3{0.7, 0.7, 1.0} * (1 - std::abs(V.y)) + glm::vec3{0.1, 0.2, 0.4} * std::abs(V.z);
	}
};

/**
	A test Phong-like material
*/
class test_material : public material
{
public:
	test_material() :
		material(false, false)
	{}

	glm::vec3 brdf(const ray_hit &hit, const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N) const override
	{
		glm::vec3 color{0.8, 0.6, 0.3};
		return color * glm::clamp(glm::dot(N, L), 0.f, 1.f);
	}
};

}