#pragma once 

#include <vector>
#include <random>
#include <chrono>
#include <iosfwd>
#include <atomic>

#include "scene.hpp"
#include "camera.hpp"
#include "ray_accelerator.hpp"
#include "containers/image.hpp"
#include "utility.hpp"

namespace rt {

/**
	Path tracing context - use one per thread. Context contains
	data and objects reused between subsequent pixel sampling
	operations.
*/
class path_tracer
{
	friend std::ostream &operator<<(std::ostream &, const path_tracer &);

public:
	path_tracer(const rt::scene &sc, rt::sampled_hdr_image &img, unsigned long seed);

	//! Samples one pixel
	glm::vec3 sample_pixel(const glm::vec2 &pixel_pos, int max_depth = 40, float survival_bias = 4.f) const;
	
	//! Samples each pixel in the image
	void sample_image(int max_depth = 40, float survival_bias = 4.0f, const std::atomic<bool> *stop_flag = nullptr);

	//! Clears the image and resets sample number
	void clear_image();

	//! Provides access to the image
	const rt::sampled_hdr_image &get_image() const
	{
		return *m_image;
	}

	//! Provides access to private random float generator
	float get_rand() const
	{
		return m_dist(m_rng);
	}

	//! Returns time elapsed for the last sample
	std::chrono::duration<double> get_last_sample_time() const
	{
		return m_t_last;
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

	//! Time taken for the last sample
	std::chrono::duration<double> m_t_last;

	//! Image data
	sampled_hdr_image *m_image;
} __attribute__((aligned(RT_CACHE_LINE_SIZE)));


extern std::ostream &operator<<(std::ostream &, const path_tracer &);

}