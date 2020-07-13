#include "path_tracer.hpp"
#include <algorithm>
#include <iostream>

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
	const int max_depth = 40;

	// Current ray
	rt::ray r = m_camera->get_ray(pixel_pos);
	glm::vec3 weight{1.f};
	float ior = 1.f;
	int depth = 0;

	//! \todo teminate rays with Russain roulette instead
	while (depth < max_depth)
	{
		hit = m_scene->cast_ray(r, *m_accelerator);
		bounce = hit.material->get_bounce(*this, hit, ior);

		// Emissive materials terminate rays
		// and contribute to the pixel through
		// ray's weight
		if (bounce.emission != glm::vec3{0.f})
		{
			pixel += weight * bounce.emission;
			break;
		}
		else
		{
			r = bounce.new_ray;
			weight *= bounce.bsdf;
			ior = bounce.ior;
			depth++;
		}
	}

	return pixel;
}

/**
	Performs one pass of sampling.
*/
void path_tracer::sample_image()
{
	auto t_start = std::chrono::high_resolution_clock::now();

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

	// Measure time
	auto t_end = std::chrono::high_resolution_clock::now();
	m_t_last = t_end - t_start;
}

void path_tracer::clear_image()
{
	std::fill(m_pixels.begin(), m_pixels.end(), glm::vec3{0.f});
}

std::ostream &operator<<(std::ostream &s, const path_tracer &pt)
{
	s << "rt::path_tracer - " << pt.get_sample_count() << " samples - " 
		<< pt.get_last_sample_time().count() << "s per sample";
	return s;
}