#include "path_tracer.hpp"
#include <algorithm>

using rt::path_tracer;

path_tracer::path_tracer(const rt::camera &cam, const rt::scene &sc, const rt::ray_accelerator &accel, int width, int height, unsigned long seed) :
	m_camera(&cam),
	m_scene(&sc),
	m_accelerator(&accel),
	m_rng(seed),
	m_dist(0.f, 1.f),
	m_pixels(width * height, glm::vec3{0.f}),
	m_resolution(width, height),
	m_sample_count(0)
{
}

glm::vec3 path_tracer::sample_pixel(const glm::vec2 &pixel_pos) const
{
	// Hit record and bounce/scatter
	rt::ray_hit hit;
	rt::ray_bounce bounce;

	// Sampled pixel
	glm::vec3 pixel{0.f};

	// Path termination conditions
	const float min_weight = 0.000;
	const int max_depth = 100;

	// Ray parameters
	rt::ray_branch current;
	current.r = m_camera->get_ray(pixel_pos);
	current.weight = glm::vec3{1.f};
	current.ior = 1.f;
	current.depth = 0;

	while (true)
	{
		hit = m_scene->cast_ray(current.r, *m_accelerator);
		bounce = hit.get_bounce(*this, current.ior);

		// Emissive materials terminate rays
		// and contribute to the pixel through
		// ray's weight
		if (bounce.emission != glm::vec3{0.f})
		{
			pixel += current.weight * bounce.emission;
			break;
		}
		else
		{
			if (current.depth > 40) break;

			// bool spawn_reflection = 
				// (glm::length(bounce.brdf * current.weight) > min_weight)
				// (current.depth + 1 < max_depth);

			// bool spawn_transmission = 
				// (glm::length(bounce.btdf * current.weight) > min_weight)
				// (current.depth + 1 < max_depth);

			float R = glm::length(bounce.brdf) / bounce.reflection_pdf;
			float T = glm::length(bounce.btdf);
			
			// No outgoing rays
			if (R + T == 0.f) break;

			float p_r = R / (R + T);
			float p_t = T / (R + T);

			// Uniform random variable
			float x = this->get_rand();

			if (x < p_r) // Sample reflection
			{
				// Only traverse reflection ray
				current.r = bounce.reflected_ray;
				current.weight *= bounce.brdf / bounce.reflection_pdf / p_r;
				current.depth++;
			}
			else // Sample transmission
			{
				// Only traverse transmission ray
				current.r = bounce.transmitted_ray;
				current.weight *= bounce.btdf / p_t;
				current.ior = bounce.transmission_ior;
				current.depth++;
			}
		}
	}

	return pixel;
}

/**
	Performs one pass of sampling.
*/
void path_tracer::sample_image()
{
	for (int y = 0; y < m_resolution.y; y++)
	{
		for (int x = 0; x < m_resolution.x; x++)
		{
			// Normalized pixel coordinates + random anti-aliasing offset
			glm::vec2 pixel_pos{
				(x + this->get_rand()) / m_resolution.x * 2.f - 1.f,
				1.f - (y + this->get_rand()) / m_resolution.y * 2.f
			};

			// Write pixel
			m_pixels[y * m_resolution.x + x] += sample_pixel(pixel_pos);
		}
	}

	m_sample_count++;
}

void path_tracer::clear_image()
{
	std::fill(m_pixels.begin(), m_pixels.end(), glm::vec3{0.f});
}