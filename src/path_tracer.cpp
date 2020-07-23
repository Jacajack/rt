#include "path_tracer.hpp"
#include <algorithm>
#include <iostream>

using rt::path_tracer;

path_tracer::path_tracer(const rt::scene &sc, rt::sampled_hdr_image &img, unsigned long seed) :
	m_camera(&sc.get_camera()),
	m_scene(&sc),
	m_accelerator(&sc.get_accelerator()),
	m_rng(seed),
	m_dist(0.f, 1.f),
	m_image(&img)
{
}

glm::vec3 path_tracer::sample_pixel(const glm::vec2 &pixel_pos, int max_depth, float survival_bias) const
{
	// Hit record and bounce/scatter
	rt::ray_hit hit;
	rt::ray_bounce bounce;

	// Sampled pixel
	glm::vec3 pixel{0.f};

	// Current ray
	rt::ray r = m_camera->get_ray(pixel_pos);
	glm::vec3 weight{1.f};
	float ior = 1.f;
	int depth = 0;

	while (depth < max_depth && weight != glm::vec3{0.f})
	{
		// Russian roulette for path termination
		float p_survive = std::min(1.f, survival_bias * std::max(weight.x, std::max(weight.y, weight.z)));
		if (get_rand() >= p_survive)
			break;

		weight /= p_survive;
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
void path_tracer::sample_image(int max_depth, float p_extinct, const std::atomic<bool> *active_flag)
{
	auto res = m_image->get_dimensions();
	auto t_start = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < res.y && (!active_flag || *active_flag); y++)
	{
		for (int x = 0; x < res.x && (!active_flag || *active_flag); x++)
		{
			// Normalized pixel coordinates + random anti-aliasing offset
			glm::vec2 pixel_pos{
				(x + this->get_rand()) / res.x * 2.f - 1.f,
				1.f - (y + this->get_rand()) / res.y * 2.f
			};

			// Write pixel
			m_image->pixel(x, y) += sample_pixel(pixel_pos, max_depth, p_extinct);
		}
	}

	m_image->add_sample();

	// Measure time
	auto t_end = std::chrono::high_resolution_clock::now();
	m_t_last = t_end - t_start;
}

void path_tracer::clear_image()
{
	m_image->clear();
}

std::ostream &operator<<(std::ostream &s, const path_tracer &pt)
{
	s << "rt::path_tracer - " << pt.get_image().get_sample_count() << " samples - " 
		<< pt.get_last_sample_time().count() << "s per sample";
	return s;
}