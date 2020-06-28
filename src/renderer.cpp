#include "renderer.hpp"

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
void renderer::sample()
{
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

			// TEMP
			// Evaluate BRDF
			glm::vec3 lum = hit.brdf->operator()({0, 1, 0}, -hit.direction, hit.normal);

			// Write pixel
			m_pixels[y * m_resolution.x + x] = 4.f * lum;	
		}
	}
}

void renderer::pixels_to_rgba(uint8_t *ptr)
{
	for (int y = 0; y < m_resolution.y; y++)
	{
		for (int x = 0; x < m_resolution.x; x++)
		{
			//! \todo tonemapping
			*ptr++ = glm::clamp(m_pixels[y * m_resolution.x + x].r * 255, 0.f, 255.f);
			*ptr++ = glm::clamp(m_pixels[y * m_resolution.x + x].g * 255, 0.f, 255.f);
			*ptr++ = glm::clamp(m_pixels[y * m_resolution.x + x].b * 255, 0.f, 255.f);
			*ptr++ = 255;
		}
	}
}