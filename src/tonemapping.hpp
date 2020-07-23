#pragma once 

#include "containers/image.hpp"

namespace rt {

static inline constexpr glm::vec3 tonemap_reinhard(const glm::vec3 &x)
{
	return x / (x + 1.f);
}

static inline constexpr glm::vec3 tonemap_uncharted(const glm::vec3 &x)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

static inline glm::vec3 tonemap_filmic(const glm::vec3 &x)
{
	constexpr glm::vec3 W{11.2f};
	constexpr glm::vec3 white_scale = 1.f / rt::tonemap_uncharted(W);
	float exposure_bias = 2.f;
	return rt::tonemap_uncharted(x * exposure_bias) * white_scale;
}

static inline glm::vec3 gamma_correction(const glm::vec3 &x)
{
	return glm::pow(x, glm::vec3{1.f / 2.2f});
}

static inline rt::rgba_pixel hdr_pixel_to_rgba(const rt::hdr_pixel &x)
{
	return rt::rgba_pixel(gamma_correction(x));
}

}