#include "ray.hpp"
#include "material.hpp"

using rt::ray_hit;

// \todo Move this into some utility header
constexpr float pi = 3.141592653589793238462643383279502884f;

rt::ray_bounce ray_hit::get_bounced_ray(float r1, float r2) const
{
	// Use r1 and r2 as polar coordinates
	float phi = r1 * 2.f * pi;
	float theta = r2 * pi * 0.5f;

	// Random ray in unit hemisphere
	glm::vec3 random_vector{
		std::sin(theta) * std::cos(phi),
		std::sin(theta) * std::sin(phi),
		std::cos(theta)
	};

	// Get random tangent and bitangent
	glm::vec3 tangent{glm::normalize(glm::cross(random_vector, this->normal))};
	glm::vec3 bitangent{glm::normalize(glm::cross(tangent, this->normal))};

	// Calculate TBN matrix
	glm::mat3 tbn_mat{
		tangent,
		bitangent,
		this->normal
	};

	// Offset between new ray's origin and surface
	constexpr float bounce_offset = 0.001f;

	// Calulate bounced ray parameters
	glm::vec3 ray_dir = tbn_mat * random_vector;
	glm::vec3 ray_origin = this->position + this->normal * bounce_offset;

	// Calculate factor
	float n_dot_ray = glm::dot(this->normal, ray_dir);
	glm::vec3 brdf{this->mat->brdf(*this, ray_dir, -this->direction, this->normal)};

	return {{ray_origin, ray_dir}, n_dot_ray * brdf};
}