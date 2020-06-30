#include "pbr_material.hpp"
#include "utility.hpp"

using rt::pbr_material;

/**
	Cook-Torrance specular BRDF + Lambertian diffuse
*/
glm::vec3 pbr_material::brdf(const ray_hit &hit, const glm::vec3 &wi) const
{
	float alpha = m_roughness * m_roughness;

	// All the vectors
	glm::vec3 N = glm::normalize(hit.normal);
	glm::vec3 wo = glm::normalize(-hit.direction);
	glm::vec3 H = glm::normalize(wi + wo);

	// Estimated surface's response at normal incidence
	glm::vec3 F0 = glm::mix(glm::vec3{0.04f}, m_color, m_metallic);

	// Specular and diffuse components
	glm::vec3 k_specular = fresnel(H, wo, F0);
	glm::vec3 k_diffuse = (1.f - k_specular) * (1.f - m_metallic);

	// k for Schlick-GGX
	float k = std::pow(m_roughness + 1.f, 2.f) / 8.f;

	// Lambert and Cook-Torrance terms
	float n_dot_l = glm::max(glm::dot(N, wi), 0.f);
	float n_dot_v = glm::max(glm::dot(N, wo), 0.f);
	glm::vec3 lambert{m_color * n_dot_l / rt::pi<>};
	glm::vec3 cook_torrance{ndf(H, N, alpha) * geometry(wi, wo, N, k) / glm::max(4.f * n_dot_l * n_dot_v, 0.001f)};	
	return k_specular * cook_torrance + k_diffuse * lambert;
}

/**
	Fresnel factor (Schlick's approximation)
*/
glm::vec3 pbr_material::fresnel(const glm::vec3 &H, const glm::vec3 &V, const glm::vec3 &F0) const
{
	return F0 + (1.f - F0) * std::pow(1.f - glm::max(glm::dot(H, V), 0.f), 5.f);
}

/**
	Sclick GGX geometry shadowing function
*/
float pbr_material::schlick_ggx(float n_dot_v, float k) const
{
	return n_dot_v / (n_dot_v * (1.f - k) + k);
}

/**
	Geometry function - Schlick GGX + Smith's method
*/
float pbr_material::geometry(const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N, float k) const
{
	float n_dot_l = glm::max(glm::dot(N, L), 0.f);
	float n_dot_v = glm::max(glm::dot(N, V), 0.f);
	return schlick_ggx(n_dot_l, k) * schlick_ggx(n_dot_v, k);
}

/**
	Trowbridge-Reitz (GGX) normal distribution function
*/
float pbr_material::ndf(const glm::vec3 &H, const glm::vec3 &N, float alpha) const
{
	float alpha_sq = alpha * alpha;
	float n_dot_h = glm::max(glm::dot(N, H), 0.f);
	float denominator = n_dot_h * n_dot_h * (alpha_sq - 1.f) + 1.f;
	return alpha_sq / (rt::pi<> * denominator * denominator);
}


rt::ray_bounce pbr_material::get_bounce(const rt::ray_hit &hit, float r1, float r2) const
{
	// Use r1 and r2 as polar coordinates
	float phi = r1 * 2.f * rt::pi<>;
	float theta = r2 * 0.5f * rt::pi<>;

	// Random ray in unit hemisphere
	glm::vec3 random_vector{
		std::sin(theta) * std::cos(phi),
		std::sin(theta) * std::sin(phi),
		std::cos(theta)
	};

	// Find axis that is not parallel to the normal
	constexpr float sqrt3 = 0.57735026919f;
	glm::vec3 axis;
	if (std::abs(hit.normal.x) < sqrt3)
		axis = {1, 0, 0};
	else if (std::abs(hit.normal.y) < sqrt3)
		axis = {0, 1, 0};
	else
		axis = {0, 0, 1};

	// Calculate tangent and bitangent
	glm::vec3 tangent{glm::cross(axis, hit.normal)};
	glm::vec3 bitangent{glm::cross(tangent, hit.normal)};

	// Calculate TBN matrix
	glm::mat3 tbn_mat{
		tangent,
		bitangent,
		hit.normal
	};

	// Offset between new ray's origin and surface
	constexpr float bounce_offset = 0.001f;

	// Calulate reflected ray
	rt::ray reflected_ray{hit.position + hit.normal * bounce_offset, tbn_mat * random_vector};

	// Scattering properties
	rt::ray_bounce bounce;
	bounce.reflected_ray = reflected_ray;
	bounce.brdf = this->brdf(hit, reflected_ray.direction) * glm::dot(hit.normal, reflected_ray.direction);
	bounce.reflection_pdf = 1.f;
	
	// Transmission
	bounce.btdf = glm::vec3{0.f};
	
	// Emission
	bounce.emission = m_emission;

	return bounce;
}