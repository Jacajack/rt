#pragma once 

#include <vector>
#include <random>
#include "scene.hpp"
#include "camera.hpp"
#include "ray_accelerator.hpp"

namespace rt {

/**
	Path tracing context - use one per thread. Context contains
	data and objects reused between subsequent pixel sampling
	operations.

	\note No need to use linear_stacks here, because the context
	are not created often
*/
class path_tracer
{
public:
	path_tracer(const rt::camera &cam, const rt::scene &sc, const rt::ray_accelerator &accel, int width, int height, unsigned long seed);

	//! Samples one pixel
	glm::vec3 sample_pixel(const glm::vec2 &pixel_pos) const;
	
	//! Samples each pixel in the stored image
	void sample_image();

	//! Clears the stored image
	void clear_image();

	//! Provides access to the stored image
	const std::vector<glm::vec3> &get_image() const
	{
		return m_pixels;
	}

	//! Returns sample count
	int get_sample_count() const
	{
		return m_sample_count;
	}

	//! Provides access to private random float generator
	float get_rand() const
	{
		return m_dist(m_rng);
	}

private:
	// Camera, scene and ray accelerator
	const rt::camera *m_camera;
	const rt::scene *m_scene;
	const rt::ray_accelerator *m_accelerator;

	//! Random number generator
	mutable std::mt19937 m_rng;
	
	//! Uniform 0-1 float distrubition
	mutable std::uniform_real_distribution<float> m_dist;

	//! Image data
	//! \todo maybe move this out?
	std::vector<glm::vec3> m_pixels;
	glm::ivec2 m_resolution;
	int m_sample_count;
};


}