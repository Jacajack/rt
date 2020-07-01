#include "pbr_material.hpp"
#include "utility.hpp"

using rt::pbr_material;

/**
	Fresnel factor
*/
static inline float fresnel_factor(const glm::vec3 &H, const glm::vec3 &V, float F0)
{
	float eta = (1.f + std::sqrt(F0)) / (1 - std::sqrt(F0));
	float c = glm::dot(H, V);
	float g = std::sqrt(eta * eta + c * c - 1);
	float gmc = g - c;
	float gpc = g + c;
	return 0.5f * (gmc / gpc) * (gmc / gpc) * (1.f + (gpc * c - 1.f) / (gmc * c + 1.f));

	// return F0 + (1.f - F0) * std::pow(1.f - glm::max(glm::dot(H, V), 0.f), 5.f);
}

/**
	Sclick GGX geometry shadowing function
*/
static inline float schlick_ggx(float n_dot_v, float k)
{
	return n_dot_v / (n_dot_v * (1.f - k) + k);
}

/**
	Geometry shadowing function - Schlick GGX + Smith's method
*/
static inline float smith_shadowing(const glm::vec3 &L, const glm::vec3 &V, const glm::vec3 &N, float k) 
{
	float n_dot_l = glm::max(glm::dot(N, L), 0.f);
	float n_dot_v = glm::max(glm::dot(N, V), 0.f);
	return schlick_ggx(n_dot_l, k) * schlick_ggx(n_dot_v, k);
}

/**
	Beckmann shadowing function
*/
static inline float beckmann_shadowing(const glm::vec3 &V, const glm::vec3 &N, float alpha)
{
	float n_dot_v = glm::max(glm::dot(N, V), 0.f);
	float c = n_dot_v / (alpha * std::sqrt(1.f - n_dot_v * n_dot_v));
	
	if (c < 1.6f)
	{
		float numerator = c * (3.535f + 2.181f * c);
		float denominator = 1.f + c * (2.276f + 2.577f * c);
		return numerator / denominator;
	}
	else
	{
		return 1;
	}
}

/**
	Trowbridge-Reitz (GGX) normal distribution function
*/
static inline float trowbridge_reitz_ggx(const glm::vec3 &H, const glm::vec3 &N, float alpha)
{
	float alpha_sq = alpha * alpha;
	float n_dot_h = glm::max(glm::dot(N, H), 0.f);
	float denominator = n_dot_h * n_dot_h * (alpha_sq - 1.f) + 1.f;
	return alpha_sq / (rt::pi<> * denominator * denominator);
}


rt::ray_bounce pbr_material::get_bounce(const rt::ray_hit &hit, float r1, float r2) const
{
	// Use r1 and r2 as polar coordinates to 
	float phi = r1 * 2.f * rt::pi<>;
	float theta = std::atan(m_alpha * std::sqrt(r2 / (1.f - r2)));

	// Outgoing light vector and normal
	glm::vec3 wo{-hit.direction};

	// The microfacet normal in tangent space
	glm::vec3 wm_local{
		std::sin(theta) * std::cos(phi),
		std::sin(theta) * std::sin(phi),
		std::cos(theta)
	};

	// Find axis that is not parallel to the normal
	glm::vec3 axis;
	if (std::abs(hit.normal.y) < 0.99)
		axis = {0, 1, 0};
	else
		axis = {1, 0, 0};

	// Construct TBN matrix to transform from tangent space to world space
	glm::vec3 tangent{glm::cross(glm::normalize(axis), hit.normal)};
	glm::vec3 bitangent{glm::cross(tangent, hit.normal)};
	glm::mat3 tbn_mat{
		tangent,
		bitangent,
		hit.normal
	};

	// Transform the microfacet normal to world space
	glm::vec3 wm = tbn_mat * wm_local;

	// Incoming light vector
	glm::vec3 wi = 2 * glm::dot(wo, wm) * wm - wo; 

	// The fresnel term determines whether the specular or diffuse
	// layer is going to be evaluated
	float F0 = glm::mix(0.04f, 1.f, m_metallic);
	float F = fresnel_factor(wm, wo, F0);

	// Trowbride-Reitz GGX
	float D = trowbridge_reitz_ggx(wm, hit.normal, m_alpha);

	// Trowbridge-Reitz PDF for the outgoing ray
	float pdf = D * glm::dot(wm, hit.normal) / 4.f / glm::dot(wm, wi);

	// Smith's geometry shadowing function
	float smith_k = std::pow(m_alpha + 1, 2.f) / 8.f;
	// float G = smith_shadowing(wi, wo, hit.normal, smith_k);
	float G = beckmann_shadowing(wo, hit.normal, m_roughness);

	// Cook-Torrance specular reflection BRDF without Fresnel factor
	glm::vec3 cook_torrance{G * D / (4.f * glm::dot(hit.normal, wi) * glm::dot(hit.normal, wo))};

	// Lambertian diffuse
	glm::vec3 lambert{m_color / rt::pi<>};

	// Reflected ray
	constexpr float bounce_offset = 0.001f;
	rt::ray reflected_ray{hit.position + hit.normal * bounce_offset, wi};

	// Test Fresnel factor against random number and decide which
	// BRDF is used
	glm::vec3 brdf;
	float r3 = drand48(); // REPLACE
	if (r3 > F)
	{
		theta = r2 * 0.5f * rt::pi<>;
		glm::vec3 wr_local{
			std::sin(theta) * std::cos(phi),
			std::sin(theta) * std::sin(phi),
			std::cos(theta)
		};
		glm::vec3 wr = tbn_mat * wr_local;

		wi = wr;
		reflected_ray = rt::ray{hit.position + hit.normal * bounce_offset, wr};

		brdf = lambert;
		pdf = 1.f / 2.f / rt::pi<>;
	}
	else
	{
		brdf = cook_torrance * glm::mix(glm::vec3{1.f}, m_color, m_metallic);
	}


	

	// Scattering properties
	rt::ray_bounce bounce;
	bounce.reflected_ray = reflected_ray;
	bounce.reflection_pdf = pdf;
	bounce.brdf = brdf * glm::max(glm::dot(hit.normal, wi), 0.f);
	
	// Transmission
	bounce.btdf = glm::vec3{0.f};
	
	// Emission
	bounce.emission = m_emission;

	return bounce;

	/*
	//! \todo FIXME
	glm::vec3 N = glm::normalize(hit.normal);
	glm::vec3 wo = glm::normalize(-hit.direction);
	float alpha = m_roughness * m_roughness;

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


	// Use r1 and r2 as polar coordinates
	float phi = r1 * 2.f * rt::pi<>;
	float theta = std::atan(alpha * std::sqrt(r2 / (1.f - r2)));

	glm::vec3 wm{
		std::sin(theta) * std::cos(phi),
		std::sin(theta) * std::sin(phi),
		std::cos(theta)
	};


	glm::vec3 wi = glm::reflect(-wo, wm);

	// Calculate microfacet normal in world space
	glm::vec3 microfacet = tbn_mat * microfacet_local;

	// PDF for polar coordinates
	// float pdf = alpha * alpha * std::cos(theta) * std::sin(theta) / (rt::pi<> * std::pow((alpha * alpha - 1.f) * std::cos(theta) + 1.f, 2.f));
	float pdf = ndf(microfacet, N, alpha) * glm::dot(microfacet, N) / 4 / glm::dot(wo, microfacet);
	//glm::vec3 wi = 2.f * glm::dot(wo, microfacet) * microfacet - wo;

	// Offset between new ray's origin and surface
	constexpr float bounce_offset = 0.001f;

	// Calulate reflected ray
	rt::ray reflected_ray{hit.position + hit.normal * bounce_offset, wi};

	glm::vec3 H = glm::normalize(reflected_ray.direction + wo);
	float n_dot_h = glm::max(glm::dot(N, H), 0.f);
	//float pdf = ndf(H, N, alpha) * n_dot_h * std::sqrt(1.f - n_dot_h * n_dot_h);

	// Scattering properties
	rt::ray_bounce bounce;
	bounce.reflected_ray = reflected_ray;
	bounce.reflection_pdf = pdf;
	bounce.brdf = this->brdf(hit, reflected_ray.direction) * glm::dot(hit.normal, reflected_ray.direction);
	bounce.reflection_pdf = 1.f;
	
	// Transmission
	bounce.btdf = glm::vec3{0.f};
	
	// Emission
	bounce.emission = m_emission;

	return bounce;
	*/
}




/**
	// Cook-Torrance specular BRDF + Lambertian diffuse
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
*/
