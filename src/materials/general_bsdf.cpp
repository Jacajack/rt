#include "general_bsdf.hpp"
#include "../path_tracer.hpp"
#include "../ray.hpp"

using rt::general_bsdf;

/**
	Fresnel factor (Schlick's approximation)
*/
static inline float F(float cos_theta, float F0)
{
	return F0 + (1.f - F0) * std::pow(1.f - std::max(cos_theta, 0.f), 5.f);

	// float eta = (1.f + std::sqrt(F0)) / (1 - std::sqrt(F0));
	// float c = cos_theta;
	// float g = std::sqrt(eta * eta + c * c - 1);
	// float gmc = g - c;
	// float gpc = g + c;
	// return 0.5f * (gmc / gpc) * (gmc / gpc) * (1.f + (gpc * c - 1.f) / (gmc * c + 1.f));
}

/**
	Fresnel factor (Schlick's approximation)
*/
static inline glm::vec3 F(float cos_theta, const glm::vec3 &F0)
{
	return F0 + (1.f - F0) * std::pow(1.f - std::max(cos_theta, 0.f), 5.f);
}

/**
	Masking function
	https://computergraphics.stackexchange.com/questions/2489/correct-form-of-the-ggx-geometry-term
*/
static inline float G1(const glm::vec3 &V, const glm::vec3 &N, const glm::vec3 &M, float alpha2)
{
	float c = glm::dot(N, V); // cos(theta_v)
	float t2 = (1.f - c * c) / c * c;
	return 2.f / (1.f + std::sqrt(1.f + alpha2 * t2)); // Skipping clamp here
}

/**
	Masking-Shadowing function

	https://twvideo01.ubm-us.net/o1/vault/gdc2017/Presentations/Hammon_Earl_PBR_Diffuse_Lighting.pdf
	https://schuttejoe.github.io/post/ggximportancesamplingpart1/
*/
static inline float G2(const glm::vec3 &wi, const glm::vec3 &wg, const glm::vec3 &wo, float alpha2)
{
	float wg_wi = glm::dot(wg, wi);
	float wg_wo = glm::dot(wg, wo);
	float denom_a = wg_wo * std::sqrt(alpha2 + (1.f - alpha2) * wg_wi * wg_wi);
	float denom_b = wg_wi * std::sqrt(alpha2 + (1.f - alpha2) * wg_wo * wg_wo);
	return 2.f * wg_wi * wg_wo / (denom_a + denom_b);
}

/**
	Heitz (2017)
*/
static inline glm::vec3 get_normal(const glm::vec3 &v, float alpha, float u1, float u2)
{
	// The stretched view vector
	glm::vec3 V{glm::normalize(v * glm::vec3{alpha, alpha, 1.f})};

	// Build tangent space
	glm::vec3 T1 = (V.z < 0.999f) ? glm::normalize(glm::cross(V, glm::vec3{0, 0, 1})) : glm::vec3{1, 0, 0};
	glm::vec3 T2{glm::cross(T1, V)};

	// Sample point
	float a = 1.f / (1.f + V.z);
	float r = std::sqrt(u1);
	float phi = (u2 < a) ? (u2 / a * rt::pi<>) : (rt::pi<> + rt::pi<> * (u2 - a) / (1.f - a));
	float P1 = r * std::cos(phi);
	float P2 = r * std::sin(phi) * (u2 < a ? 1.f : V.z);

	// Compute normal
	glm::vec3 N = P1 * T1 + P2 * T2 + std::sqrt(std::max(0.f, 1.f - P1 * P1 - P2 * P2)) * V;

	// Return unstretched normal
	return glm::normalize(glm::vec3{alpha * N.x, alpha * N.y, glm::max(0.f, N.z)});
}

rt::ray_bounce general_bsdf::get_bounce(const rt::path_tracer &ctx, const ray_hit &hit, float ior) const
{
	// Alpha is roughness squared
	float alpha = this->roughness * this->roughness;
	float alpha2 = alpha * alpha;

	// Directions
	const glm::vec3 wo = -hit.direction;
	glm::vec3 wg = hit.normal;
	glm::vec3 wm, wi;

	// IOR for the new ray (doesn't change by default) and eta
	float new_ior = ior;
	float eta = 1.f / this->ior;

	// Ignore back faces 
	if (glm::dot(wo, wg) < 0.f)
	{
		// Unless there's chance for transmission
		if (this->transmission != 0.f)
		{
			wg = -wg;
			new_ior = 1.f; // TODO: fix this and pop IOR from IOR stack
			eta = this->ior;
		}
		else
		{
			rt::ray_bounce bounce;
			bounce.bsdf = glm::vec3{0.f};
			bounce.ior = ior;
			bounce.new_ray = rt::ray{hit.position, wg};
			bounce.emission = this->emission;
			return bounce;
		}
	}

	// Build TBN matrix
	glm::vec3 tangent{glm::normalize(glm::cross(wo, wg))};
	glm::vec3 bitangent{glm::normalize(glm::cross(tangent, wg))};
	glm::mat3 tbn_mat{
		tangent,
		bitangent,
		wg
	};
	glm::mat3 inv_tbn_mat{glm::transpose(tbn_mat)};

	// Two random variables for sampling
	float r1 = ctx.get_rand();
	float r2 = ctx.get_rand();

	// Sample the distribution of visible normals
	// and get microfacet normal
	wm = tbn_mat * get_normal(inv_tbn_mat * wo, alpha, r1, r2);
	
	// G2 = G1(wi, wg, wm) * G1(wo, wg, wm)
	// weight = G2 / G1(wi, wg, wm) = G1(wo, wg, wm)
	float weight = G1(wo, wg, wm, alpha2);

	// Calculate Fresnel term and determine whether the ray bounce is specular
	// (metals are reflective only)
	float F0 = (this->ior - ior) / (this->ior + ior);
	F0 = F0 * F0;
	F0 = glm::mix(F0, 1.f, metallic);

	if (ctx.get_rand() < F(glm::dot(wo, wm), F0))
	// if(0)
	{
		wi = glm::reflect(-wo, wm);

		// Ignore light directions from below the surface
		if (glm::dot(wg, wi) <= 0.f)
		{
			rt::ray_bounce bounce;
			bounce.bsdf = glm::vec3{0.f};
			bounce.emission = this->emission;
			bounce.ior = new_ior;
			return bounce;
		}

		// Reflected light (tinted if metallic)
		rt::ray_bounce bounce;
		bounce.bsdf = weight * glm::mix(glm::vec3{1.f}, base_color, metallic);
		bounce.ior = new_ior;
		bounce.new_ray = rt::ray{hit.position + wg * 0.0001f, wi};
		bounce.emission = this->emission;
		return bounce;
	}
	else
	{
		// Transmission / diffuse
		if (this->transmission != 0.f && ctx.get_rand() < this->transmission)
		{
			wi = {glm::refract(-wo, wm, eta)};
			
			if (glm::length(wi) != 0.f)
			{
				// Transmission
				rt::ray_bounce bounce;
				bounce.bsdf = this->base_color * weight;
				bounce.ior = new_ior;
				bounce.new_ray = rt::ray{hit.position - wg * 0.0001f, wi};
				bounce.emission = this->emission;
				return bounce;
			}
			else
			{
				// Total internal reflection
				wi = glm::reflect(-wo, wm);

				rt::ray_bounce bounce;
				bounce.bsdf = glm::vec3{weight};
				bounce.ior = ior;
				bounce.new_ray = rt::ray{hit.position + wg * 0.0001f, wi};
				bounce.emission = this->emission;
				return bounce;
			}
		}
		else
		{
			// Recalculate new ray from cosine-weighted distribution
			float cos_theta = std::sqrt(r1);
			float sin_theta = std::sqrt(1.f - r1);
			glm::vec3 wi_local{
				sin_theta * std::cos(2.f * rt::pi<> * r2),
				sin_theta * std::sin(2.f * rt::pi<> * r2),
				cos_theta
			};
			wi = tbn_mat * wi_local;

			// Lambert
			rt::ray_bounce bounce;
			bounce.bsdf = base_color * weight;
			bounce.ior = ior;
			bounce.new_ray = rt::ray{hit.position + wg * 0.0001f, wi};
			bounce.emission = this->emission;
			return bounce;
		}	
	}
}