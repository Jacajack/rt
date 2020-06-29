#include "renderer.hpp"
#include <random>

using rt::renderer;

renderer::renderer(const scene &sc, const camera &cam, const glm::ivec2 &resolution) :
	m_scene(&sc),
	m_camera(&cam),
	m_resolution(resolution),
	m_pixels(m_resolution.x * m_resolution.y, {0, 0, 0})
{
}

/**
	Performs one pass of sampling.

	In the future we will be having more fun in this function
*/
void renderer::sample(int seed)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	std::vector<rt::ray_bounce> bounces;
	// std::vector<rt::ray_hit> hits;
	bounces.reserve(20);
	// hits.reserve(20);

	for (int y = 0; y < m_resolution.y; y++)
	{
		for (int x = 0; x < m_resolution.x; x++)
		{
			// Normalized pixel coordinates
			float nx = (x + 0.5f) / m_resolution.x * 2.f - 1.f;
			float ny = 1.f - (y + 0.5f) / m_resolution.y * 2.f;
			glm::vec2 pixel_pos{nx, ny};

			// Cast ray
			rt::ray pixel_ray = m_camera->get_ray(pixel_pos);
			rt::ray_hit hit = m_scene->cast_ray(pixel_ray);

			// Clear bounces list
			bounces.clear();

			// Bounce rays
			while (bounces.size() < 10 && !hit.mat->is_emissive())
			{
				// Get bounced ray
				bounces.push_back(hit.get_bounced_ray(dist(rng), dist(rng)));

				// Cast the bounced ray
				hit = m_scene->cast_ray(bounces.back().new_ray);
			}

			// TEMP Only count rays if the last hit was emissive
			if (!hit.mat->is_emissive()) continue;

			// Calculate contribution
			glm::vec3 factor{1, 1, 1};
			for (const auto &b : bounces)
				factor *= b.factor;

			// Write pixel
			m_pixels[y * m_resolution.x + x] += factor; 
		}
	}
}

void renderer::pixels_to_rgba(uint8_t *ptr)
{
	for (int y = 0; y < m_resolution.y; y++)
	{
		for (int x = 0; x < m_resolution.x; x++)
		{
			// Reinhard tonemapping and sRGB correction
			glm::vec3 pix =	m_pixels[y * m_resolution.x + x];
			pix = pix / (pix + 1.f);
			pix = glm::pow(pix, glm::vec3{1.f / 2.2f});

			*ptr++ = pix.r * 255.99f;
			*ptr++ = pix.g * 255.99f;
			*ptr++ = pix.b * 255.99f;
			*ptr++ = 255;
		}
	}
}