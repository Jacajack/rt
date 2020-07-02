#include "renderer.hpp"
#include <random>

using rt::renderer;

renderer::renderer(const scene &sc, const camera &cam, const glm::ivec2 &resolution, const ray_accelerator &accel) :
	m_scene(&sc),
	m_camera(&cam),
	m_accelerator(&accel),
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
	bounces.reserve(20);

	for (int y = 0; y < m_resolution.y; y++)
	{
		for (int x = 0; x < m_resolution.x; x++)
		{
			// Normalized pixel coordinates + random anti-aliasing offset
			glm::vec2 pixel_pos{
				(x + dist(rng)) / m_resolution.x * 2.f - 1.f,
				1.f - (y + dist(rng)) / m_resolution.y * 2.f
			};

			// Cast ray
			rt::ray pixel_ray = m_camera->get_ray(pixel_pos);
			rt::ray_hit hit = m_scene->cast_ray(pixel_ray, *m_accelerator);

			// Clear bounces list
			bounces.clear();

			// Bounce rays
			do
			{
				// Get bounced ray
				bounces.push_back(hit.get_bounce(dist(rng), dist(rng)));

				// Cast the bounced ray
				hit = m_scene->cast_ray(bounces.back().reflected_ray, *m_accelerator);
			}
			while (bounces.size() < 5 && bounces.back().emission == glm::vec3{0.f});

			// Skip if the last hit was not emissive
			if (bounces.back().emission == glm::vec3{0.f}) continue;
			
			// Integrate radiance from all bounces
			glm::vec3 radiance{0.f};
			for (int i = bounces.size() - 1; i >= 0; i--)
			{
				const auto &b = bounces.at(i);
				radiance = radiance * b.brdf / b.reflection_pdf + b.emission;
			}

			// Write pixel
			m_pixels[y * m_resolution.x + x] += radiance;
		}
	}

	m_samples++;
}

void renderer::pixels_to_rgba(uint8_t *ptr)
{
	for (int y = 0; y < m_resolution.y; y++)
	{
		for (int x = 0; x < m_resolution.x; x++)
		{
			// Reinhard tonemapping and sRGB correction
			glm::vec3 pix =	m_pixels[y * m_resolution.x + x] / float(m_samples);
			pix = pix / (pix + 1.f);
			pix = glm::pow(pix, glm::vec3{1.f / 2.2f});

			*ptr++ = pix.r * 255.99f;
			*ptr++ = pix.g * 255.99f;
			*ptr++ = pix.b * 255.99f;
			*ptr++ = 255;
		}
	}
}