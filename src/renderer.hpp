#pragma once
#include <vector>
#include <random>

#include "camera.hpp"
#include "scene.hpp"
#include "ray_accelerator.hpp"

namespace rt {

/**
	Takes a camera and a scene and produces image
*/
class renderer
{
public:
	struct path_tracing_context;

	renderer(const scene &sc, const camera &cam, const ray_accelerator &accel);

	glm::vec3 sample_pixel(path_tracing_context &ctx, const glm::vec2 &pixel_pos);
	void sample_image(path_tracing_context &ctx);

	void pixels_to_rgba(uint8_t *ptr);

private:
	const scene *m_scene;
	const camera *m_camera;
	const ray_accelerator *m_accelerator;

};

/**
	Path tracing context - use one per thread. Context contains
	data and objects reused between subsequent pixexl sampling
	operations (including image buffer).

	\note No need to use linear_stacks here, because the context
	are not created often
*/
struct renderer::path_tracing_context
{
	path_tracing_context(int width, int height, unsigned long seed);

	//! Random number generator
	std::mt19937 rng;
	
	//! Uniform 0-1 float distrubition
	std::uniform_real_distribution<float> dist;

	//! Ray branching points
	std::vector<rt::ray_branch> branch_stack;

	//! Image data
	std::vector<glm::vec3> pixels;
	const glm::ivec2 resolution;
	int sample_count;
};

}