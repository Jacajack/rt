#include "renderer.hpp"
#include <random>
#include "linear_stack.hpp"

using rt::renderer;

renderer::renderer(const scene &sc, const camera &cam, const ray_accelerator &accel) :
	m_scene(&sc),
	m_camera(&cam),
	m_accelerator(&accel)
{
}

glm::vec3 renderer::sample_pixel(path_tracing_context &ctx, const glm::vec2 &pixel_pos)
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
		bounce = hit.get_bounce(current.ior, ctx.dist(ctx.rng), ctx.dist(ctx.rng));

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
			float x = ctx.dist(ctx.rng);

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

	In the future we will be having more fun in this function
*/
void renderer::sample_image(path_tracing_context &ctx)
{
	for (int y = 0; y < ctx.resolution.y; y++)
	{
		for (int x = 0; x < ctx.resolution.x; x++)
		{
			// Normalized pixel coordinates + random anti-aliasing offset
			glm::vec2 pixel_pos{
				(x + ctx.dist(ctx.rng)) / ctx.resolution.x * 2.f - 1.f,
				1.f - (y + ctx.dist(ctx.rng)) / ctx.resolution.y * 2.f
			};

			// Write pixel
			ctx.pixels[y * ctx.resolution.x + x] += sample_pixel(ctx, pixel_pos);
		}
	}

	ctx.sample_count++;
}

renderer::path_tracing_context::path_tracing_context(int width, int height, unsigned long seed) :
	rng(seed),
	dist(0.f, 1.f),
	pixels(width * height, glm::vec3{0.f}),
	resolution(width, height),
	sample_count(0)
{
}